#ifndef _REFLECTION_HPP
#define _REFLECTION_HPP

#pragma once

#include <stdexcept>
#include <sstream>
#include <boost/any.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/foreach.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>

#include "reflection_helpertypes.hpp"
#include "reflection_call.hpp"
#include "reflection_method.hpp"
#include "reflection_constructor.hpp"
#include "reflection_class.hpp"
#include "reflection_module.hpp"

namespace reflection
{
using namespace std;

template < typename T > class __TrueKeeper
{
public:
    typedef T TYPE;
protected:
    T* value;
public:
    __TrueKeeper (T* value): value (value)
    {
    }
    T* operator-> ()
    {
        return value;
    }
    operator bool ()
    {
        return true;
    }
};
}

using reflection::Module;
using reflection::Parameters_t;

template < typename class_t, typename ret_t, typename ... arg_t > reflection::Method < class_t, ret_t, arg_t ... >makeMethodType (ret_t (class_t::*)(arg_t ...));
template < typename class_t, typename ret_t, typename ... arg_t > reflection::ConstMethod < class_t, ret_t, arg_t ... >makeMethodType (ret_t (class_t::*)(arg_t ...) const);
#define export_module(x) if (reflection::__TrueKeeper<reflection::Module> __module__ = &x)
#define insert_class(x) if (reflection::__TrueKeeper<reflection::Class<x> > __class__ = (reflection::Class<x>*)__module__->createClass<x>(#x))
#define __defaultconstructor() __class__->registerConstructor__< decltype(__class__)::TYPE::TYPE>()
#define __constructor(...) __class__->registerConstructor__< decltype(__class__)::TYPE::TYPE , __VA_ARGS__ >()
#define __method(x,...) __class__->getMethods().insert(std::make_pair(#x, boost::shared_ptr<reflection::IMethod>(new (decltype(makeMethodType(&decltype(__class__)::TYPE::TYPE::x)))(&decltype(__class__)::TYPE::TYPE::x,  #x, ## __VA_ARGS__))))

#endif
