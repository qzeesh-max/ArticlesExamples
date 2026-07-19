#ifndef HPP_VIRTUAL_METHOD_HELPER
#define HPP_VIRTUAL_METHOD_HELPER

#include <stddef.h>

namespace virtual_method_helper
{

namespace impl
{
template <typename Class, typename Ret, typename...Arguments>
struct Decomposition
{
	using ClassType = Class;
	using NonMemberFn =  Ret (*)(Class*, Arguments...);
	using ReturnType = Ret;

	template <typename U>
	struct Rebind
        {
		typedef Ret (U::* result)(Arguments...);
	};
};

template <typename Class, typename Ret, typename...Arguments>
auto decompositionHelper(Ret (Class::*member)(Arguments...)) -> Decomposition<Class, Ret, Arguments...>;
}


template <typename T, typename Member>
auto getResolvedPointer(T* object, Member m)
{
	using namespace impl;

	typedef typename decltype(decompositionHelper(m))::ClassType* MemberClassTypePtr;
	typedef typename decltype(decompositionHelper(m))::NonMemberFn NonMemberFn;
	typedef typename decltype(decompositionHelper(m))::template Rebind<T>::result ReboundMember;

	ReboundMember rm = static_cast<ReboundMember>(m);

	if (object && rm!=nullptr)
	{
		union MemberFunctionPtr
		{	
			ReboundMember  memberPtr;
			struct
			{
				ptrdiff_t memberFunctionPtrOffset;
				ptrdiff_t vmTableOffset;
			};
		} member;

		static_assert(sizeof(ReboundMember) == sizeof(member), "This function requires virtual methods as second argument");

		member.memberPtr = rm;

#if defined(__aarch64__) || defined(__arm64__)
		ptrdiff_t vtableOffset = member.memberFunctionPtrOffset;
		ptrdiff_t thisAdj = member.vmTableOffset >> 1;
#else
		ptrdiff_t vtableOffset = member.memberFunctionPtrOffset - 1;
		ptrdiff_t thisAdj = member.vmTableOffset;
#endif

		return reinterpret_cast<NonMemberFn>(
						     *reinterpret_cast<char**>(
						     *reinterpret_cast<char**>(reinterpret_cast<char*>(object) + thisAdj)
				                     + vtableOffset));

	}

	return (NonMemberFn)nullptr;
}

}

#endif
