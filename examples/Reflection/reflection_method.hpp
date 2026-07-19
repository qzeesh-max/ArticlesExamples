#ifndef _REFLECTION_METHOD
#define _REFLECTION_METHOD

#pragma once

namespace reflection
{
class IMethod
{
public:
    virtual runtime_type_details_t getReturnType () = 0;
    virtual const ParameterDefinitions_t& getParameterDefinitions () = 0;
    virtual const ParameterNames_t getParameterNames () = 0;
    virtual const string& getMethodName () = 0;
    virtual void call (object_t object, boost::any& returnValue, Parameters_t& parameters) = 0;
    virtual void call (object_t object, Parameters_t& parameters) = 0;

    virtual ~IMethod()
    {
    }
};


template <class method_t, class class_t, typename ret_t, typename ... arg_t> class MethodBase: public IMethod
{
protected:
    method_t method;
    string methodName;
    ParameterDefinitions_t parameterDefinitions;
    ParameterNames_t parameterNames;
    template < int count > void addParameters () {  }
    template < int count, typename T, typename ... _arg_t > void addParameters ()
    {
        parameterDefinitions.push_back (runtime_type_details_t::of<T>());
        stringstream stm;
        stm << "_" << parameterDefinitions.size ();
        parameterNames.push_back (stm.str ());
        addParameters < count - 1, _arg_t ... > ();
    }
    template < typename ... _arg_t > void nameParameters (ParameterNames_t::iterator& it, _arg_t ... names);
    void nameParameters (ParameterNames_t::iterator& it) { }
    template < typename T, typename ... _arg_t > void nameParameters (ParameterNames_t::iterator& it, T name, _arg_t ... names)
    {
        if (it != parameterNames.end ())
        {
            (*it) = name;
            nameParameters (++it, names ...);
        }
    }

public:
    template < typename ... name_t > MethodBase (method_t _method, const string& _methodName, name_t ... names):
        method (_method), methodName (_methodName)
    {
        addParameters < sizeof ... (arg_t), arg_t ... >();
        auto it = parameterNames.begin ();
        nameParameters (it, names ...);
    }
    virtual runtime_type_details_t getReturnType ()
    {
        return runtime_type_details_t::of<ret_t>();
    }
    virtual const ParameterDefinitions_t& getParameterDefinitions ()
    {
        return parameterDefinitions;
    }
    virtual const ParameterNames_t getParameterNames ()
    {
        return parameterNames;
    }
    virtual const string& getMethodName ()
    {
        return methodName;
    }
    virtual void call (object_t object, boost::any& returnValue, Parameters_t& parameters)
    {

        if (parameters.size () != sizeof ... (arg_t)) throw logic_error ("Parameter count mismatch");

        if (!boost::is_same < void, ret_t >::value)
        {
            Parameters_t::iterator it = parameters.begin (), end = parameters.end ();
            ParameterDefinitions_t::const_iterator pit = parameterDefinitions.begin ();
            ParameterNames_t::const_iterator nit = parameterNames.begin ();
            reflection::call < sizeof ... (arg_t), class_t, ret_t, arg_t ... >(object, method, returnValue, pit, nit, it, end);
        }
        else
        {
            throw logic_error ("Return type mismatch");
        }

    }
    virtual void call (object_t object, Parameters_t& parameters)
    {
        if (parameters.size () != sizeof ... (arg_t)) 
		throw logic_error ("Parameter count mismatch");

     
         Parameters_t::iterator it = parameters.begin (), end = parameters.end ();
         ParameterDefinitions_t::const_iterator pit = parameterDefinitions.begin ();
         ParameterNames_t::const_iterator nit = parameterNames.begin ();
         reflection::call < sizeof ... (arg_t), class_t, ret_t, arg_t ... >(object, method, pit, nit, it, end);
    }

};

template < class class_t, typename ret_t, typename ... arg_t > class Method: 
	public MethodBase<ret_t (class_t::*)(arg_t...), class_t, ret_t, arg_t...>
{
	protected:
		typedef ret_t (class_t::* method_t)(arg_t...);

	public:
	    template < typename ... name_t > Method (method_t _method, const string& _methodName, name_t ... names):
		MethodBase<method_t,class_t,ret_t,arg_t...>(_method, _methodName, names...)
	{
	}
};



template < class class_t, typename ret_t, typename ... arg_t > class ConstMethod:
	public MethodBase<ret_t (class_t::*)(arg_t...) const, class_t, ret_t, arg_t...> 
{
	protected:
		typedef ret_t (class_t::* method_t)(arg_t...) const;

	public:
	    template < typename ... name_t > ConstMethod (method_t _method, const string& _methodName, name_t ... names):
		MethodBase<method_t,class_t,ret_t,arg_t...>(_method, _methodName, names...)
	{
	}

};

};

#endif
