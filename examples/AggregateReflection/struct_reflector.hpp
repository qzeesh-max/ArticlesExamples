#ifndef HEADER_STRUCT_REFLECTOR_H
#define HEADER_STRUCT_REFLECTOR_H

#if __cplusplus < 201703L
  #error "C++17 or later is required!"
#else

#include <utility>
#include <type_traits>
#include <tuple>
#include <string_view>
#include <array>

#ifndef __cpp_consteval
#include <string>
#include <typeinfo>
#include <cxxabi.h>
#include <unordered_map>
#endif

#include "repeat_macro.hpp"

namespace impl {

// The wildcard type provided to allow searching for number of elements allowed for aggregate initialization. 
struct SearcherType {
    template <typename T>
    operator T&() const; 
};

/*
   Meta function to allow figuring out how many elements are allowed in aggregate initialization of a structure
*/

// =====================================================================
// 1. SFINAE Predicate: Can T be initialized with N arguments?
// =====================================================================
template <typename T, typename IndexSeq, typename = void>
struct can_init_with_n_impl : std::false_type {};

template <typename T, std::size_t... Is>
struct can_init_with_n_impl<T, std::index_sequence<Is...>,
    std::void_t<decltype(T{ (void(Is), SearcherType{})... })>> 
    : std::true_type {};

template <typename T, std::size_t N>
struct can_init_n : can_init_with_n_impl<T, std::make_index_sequence<N>> {};


// =====================================================================
// 2. Binary Search Phase Metafunction
// =====================================================================
template <typename T, std::size_t Low, std::size_t High, bool IsDone = (Low == High)>
struct BinaryFieldCountSearchPhase {
    // Bias the midpoint up: Mid = Low + (High - Low + 1) / 2
    // This prevents infinite recursion when evaluating NextLow = Mid
    static constexpr std::size_t Mid = Low + (High - Low + 1) / 2;
    static constexpr bool MidIsValid = can_init_n<T, Mid>::value;

    // If Mid is valid, valid boundary is at least Mid. Otherwise, strictly less.
    static constexpr std::size_t NextLow = MidIsValid ? Mid : Low;
    static constexpr std::size_t NextHigh = MidIsValid ? High : Mid - 1;

    static constexpr std::size_t value = BinaryFieldCountSearchPhase<T, NextLow, NextHigh>::value;
};

// Base case: Low == High
template <typename T, std::size_t Low, std::size_t High>
struct BinaryFieldCountSearchPhase<T, Low, High, true> {
    static constexpr std::size_t value = Low;
};


// =====================================================================
// 3. Exponential Phase Metafunction
// =====================================================================
template <typename T, std::size_t Low, std::size_t High, bool HighIsValid = can_init_n<T, High>::value>
struct ExponentialFieldCountSearchPhase;

// True branch: High is valid, so we store High as Low, and double High
template <typename T, std::size_t Low, std::size_t High>
struct ExponentialFieldCountSearchPhase<T, Low, High, true> {
    static constexpr std::size_t value = ExponentialFieldCountSearchPhase<T, High, High * 2>::value;

};

// False branch: High is invalid, trigger binary search between Low and High - 1
template <typename T, std::size_t Low, std::size_t High>
struct ExponentialFieldCountSearchPhase<T, Low, High, false> {
    static constexpr std::size_t value = BinaryFieldCountSearchPhase<T, Low, High - 1>::value;
};


// =====================================================================
// 4. Entry Point Metafunction
// =====================================================================
template <typename T, bool FailsOnOne = !can_init_n<T, 1>::value>
struct GetStructFieldCount_impl;

// If function returns false on 1, it returns 0
template <typename T>
struct GetStructFieldCount_impl<T, true> {
    static constexpr std::size_t value = 0;

    static_assert(std::is_aggregate_v<T>, "Structure must be an aggregate to use this feature.");
};

// Otherwise, it starts the exponential search with Low=1, High=2
template <typename T>
struct GetStructFieldCount_impl<T, false> {
    static constexpr std::size_t value = ExponentialFieldCountSearchPhase<T, 1, 2>::value;

    static_assert(std::is_aggregate_v<T>, "Structure must be an aggregate to use this feature.");
};

} // namespace impl

// Actual Meta-function 
template <typename T>
constexpr std::size_t GetStructFieldCount = impl::GetStructFieldCount_impl<T>::value;


template <typename T>
inline
auto
struct_to_tuple(T& value)
{
	constexpr std::size_t FieldCount = GetStructFieldCount<T>;

#       define varcreate(x) MACRO_PASTE(v, x) 
#	define select(x) if constexpr(x == FieldCount) {\
		auto & [ REPEAT_MACRO_COMMA(varcreate, x) ] = value;\
		return std::tie(REPEAT_MACRO_COMMA(varcreate, x));\
	}

	REPEAT_MACRO(select, 255);

	static_assert(FieldCount <= 255, "cannot handle struct with more than 256 fields");

#	undef varcreate
#	undef select
}

#ifdef __cpp_consteval
template <typename T>
extern const T FakeObject;

template <auto T>
inline
consteval
std::string_view
get_field_name_helper()
{
	std::string_view name = __PRETTY_FUNCTION__;

    size_t start = name.find("FakeObject.");
    if (start != std::string_view::npos) {
        start += 11;
        size_t end = name.find_first_of("] ;)", start);
        if (end != std::string_view::npos) {
            return name.substr(start, end - start);
        }
    }

	start = name.find("& FakeObject<") + 7;
	start = name.find("::", start) + 2;
	size_t end = name.find_last_of(')');
   	
	return name.substr(start, end - start);
}

template <typename T>
inline
consteval
auto
get_field_names()
{
	constexpr std::size_t FieldCount = GetStructFieldCount<T>;

#       define varcreate(x) MACRO_PASTE(v, x) 
#	define getname(x) get_field_name_helper<&MACRO_PASTE(v, x)>()
#	define select(x) if constexpr(x == FieldCount) {\
		auto & [ REPEAT_MACRO_COMMA(varcreate, x) ] = FakeObject<T>;\
		return std::array<std::string_view, FieldCount> { REPEAT_MACRO_COMMA(getname, x) };\
	}

	REPEAT_MACRO(select, 255);

	static_assert(FieldCount <= 255, "cannot handle struct with more than 256 fields");
#	undef varcreate
#	undef select
#	undef getname
}
#else

template <typename T>
inline
auto
get_field_names()
{
	constexpr std::size_t FieldCount = GetStructFieldCount<T>;
	int status;
	size_t fieldIndex = 0;
	const T* fakeObject = nullptr;

#       define varcreate(x) MACRO_PASTE(v, x) 
#	define getname(x)  "Field [" + std::to_string(fieldIndex++) + "] @ " + std::to_string(reinterpret_cast<ptrdiff_t>(&MACRO_PASTE(v, x)))\
        + " : " + abi::__cxa_demangle(typeid(MACRO_PASTE(v,x)).name(), nullptr, nullptr, &status) 
#	define select(x) if constexpr(x == FieldCount) {\
		auto & [ REPEAT_MACRO_COMMA(varcreate, x) ] = *fakeObject;\
		return std::array<std::string, FieldCount> { REPEAT_MACRO_COMMA(getname, x) };\
	}

	REPEAT_MACRO(select, 255);

	static_assert(FieldCount <= 255, "cannot handle struct with more than 256 fields");
#	undef varcreate
#	undef select
#	undef getname
}

#endif

template <typename T, typename Functor>
inline
void
for_each_member(T&& value, Functor&& functor)
{
	static const auto fieldNames = get_field_names<T>();
	constexpr std::size_t FieldCount = GetStructFieldCount<T>;
	size_t i = 0;

#       define callfunctor(x) functor(fieldNames[i++], MACRO_PASTE(v, x))
#       define varcreate(x) MACRO_PASTE(v, x) 
#	define select(x) if constexpr(x == FieldCount) {\
		auto & [ REPEAT_MACRO_COMMA(varcreate, x) ] = value;\
		REPEAT_MACRO_COMMA(callfunctor, x);\
		return;\
	}

	REPEAT_MACRO(select, 255);

	static_assert(FieldCount <= 255, "cannot handle struct with more than 256 fields");

#	undef varcreate
#	undef select
#       undef callfunctor
	
}

template <typename T, typename Functor>
inline
void
for_each_member_value(T&& value, Functor&& functor)
{
	constexpr std::size_t FieldCount = GetStructFieldCount<T>;

#       define callfunctor(x) functor(MACRO_PASTE(v, x))
#       define varcreate(x) MACRO_PASTE(v, x) 
#	define select(x) if constexpr(x == FieldCount) {\
		auto & [ REPEAT_MACRO_COMMA(varcreate, x) ] = value;\
		REPEAT_MACRO_COMMA(callfunctor, x);\
		return;\
	}

	REPEAT_MACRO(select, 255);

	static_assert(FieldCount <= 255, "cannot handle struct with more than 256 fields");

#	undef varcreate
#	undef select
#       undef callfunctor
	
}


#endif
#endif // !defined(HEADER_STRUCT_REFLECTOR_H)
