#pragma once

#include <type_traits>
#include <tuple>

template <auto...PTM>
struct FieldApplicator;

template <auto FIRST_PTM>
struct FieldApplicator<FIRST_PTM>
{
    static_assert(std::is_member_pointer<decltype(FIRST_PTM)>::value, "Argument must be pointer to member");

    template <typename Stream, typename Object>
    static auto& write(Stream& s, const Object& o)
    {
        return s.write(o, o.*FIRST_PTM);
    }

    template <typename Stream, typename Object>
    static auto& read(Stream& s, Object& o)
    {
        return s.read(o, o.*FIRST_PTM);
    }


    template <typename Lambda, typename Object>
    static decltype(auto) apply_as_tie(const Lambda& lambda, Object o1, Object o2)
    {
        return lambda(std::tie(o1.*FIRST_PTM), std::tie(o2.*FIRST_PTM));
    }

    template <typename Lambda, typename Object>
    static decltype(auto) apply_as_tuple(const Lambda& lambda, const Object& o1, const Object& o2)
    {
        return lambda(std::make_tuple(o1.*FIRST_PTM), std::make_tuple(o2.*FIRST_PTM));
    }

};

template <auto FIRST_PTM, auto...PTM>
struct FieldApplicator<FIRST_PTM, PTM...> : FieldApplicator<PTM...>
{
    using BASE =  FieldApplicator<PTM...>;

    static_assert(std::is_member_pointer<decltype(FIRST_PTM)>::value, "Argument must be pointer to member");

    template <typename Stream, typename Object>
    static auto& write(Stream& s, const Object& o)
    {
        return BASE::write(s.write(o, o.*FIRST_PTM), o);
    }

    template <typename Stream, typename Object>
    static auto& read(Stream& s, Object& o)
    {
        return BASE::read(s.read(o, o.*FIRST_PTM), o);
    }

    template <typename Lambda, typename Object>
    static decltype(auto) apply_as_tie(const Lambda& lambda, Object o1, Object o2)
    {
        return lambda(std::tie(o1.*FIRST_PTM, o1.*PTM...), std::tie(o2.*FIRST_PTM, o2.*PTM...));
    }

    template <typename Lambda, typename Object>
    static decltype(auto) apply_as_tuple(const Lambda& lambda, const Object& o1, const Object& o2)
    {
        return lambda(std::make_tuple(o1.*FIRST_PTM, o1.*PTM...), std::make_tuple(o2.*FIRST_PTM, o2.*PTM...));
    }

};


