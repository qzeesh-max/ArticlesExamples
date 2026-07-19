#ifndef _REFLECTION_CLASS
#define _REFLECTION_CLASS

#pragma once

namespace reflection
{


typedef map < string, boost::shared_ptr<IMethod> > MethodsMap_t;

class IClass
{
public:
    virtual void registerConstructor (ParameterDefinitions_t& parameterDefinitions, IConstructor* constructor) = 0;

protected:
    template < int count > void registerConstructor__ (ParameterDefinitions_t& parameterDefinitions, IConstructor* constructor)
    {
        registerConstructor (parameterDefinitions, constructor);
    }
    template < int count, typename T, typename ... arg_t > void registerConstructor__ (ParameterDefinitions_t& parameterDefinitions, IConstructor* constructor)
    {
        parameterDefinitions.push_back (runtime_type_details_t::of<T>());

        registerConstructor__ < count - 1, arg_t ... > (parameterDefinitions, constructor);
    }    

public:
    virtual const string& getClassName () = 0;
    virtual IMethod* getMethod (const string& methodName) = 0;
    virtual object_t construct (Parameters_t& parameters) = 0;
    virtual MethodsMap_t& getMethods () = 0;
    virtual void destroy (object_t object) = 0;

    virtual ~IClass()
    {
    }




    template <typename class_t, typename ... arg_t > void registerConstructor__ ()
    {
        ParameterDefinitions_t parameterDefinitions;

        registerConstructor__ < sizeof ... (arg_t), arg_t ... >(parameterDefinitions, new Constructor<class_t>());
    }
};
typedef map < ParameterDefinitions_t, boost::shared_ptr<IConstructor> > ConstructorsList_t;

template < typename T > class Class: public IClass
{
public:
    typedef T TYPE;
protected:
    string className;
    MethodsMap_t Methods;
    ConstructorsList_t Constructors;

public:
    Class (const string& _className):
        className (_className) { }

    virtual const string& getClassName ()
    {
        return className;
    }

    virtual IMethod* getMethod (const string& methodName)
    {
        MethodsMap_t::const_iterator it = Methods.find (methodName);

        if (it != Methods.end ()) 
		return it->second.get();

        return NULL;
    }

    virtual object_t construct (Parameters_t& parameters)
    {
                    ParameterDefinitions_t parameterDefinitions;

                    BOOST_FOREACH (typed_reference_t parameter, parameters)
                    {
                        parameterDefinitions.push_back (parameter.type ());
                    }
 
		    ConstructorsList_t::const_iterator it;

		    ConstructorsList_t::const_iterator beg = Constructors.begin();
		    auto f = beg->first;
		

		    if ((it = Constructors.find (parameterDefinitions)) == Constructors.end ()) 
			throw logic_error ("No such constructor is registered");

		    return it->second->construct(parameters);

    }
    virtual MethodsMap_t& getMethods ()
    {
        return Methods;
    }

    virtual void registerConstructor (ParameterDefinitions_t& parameterDefinitions, IConstructor* constructor)
    {
        Constructors.insert (std::make_pair(parameterDefinitions, boost::shared_ptr<IConstructor>(constructor)));
    }
    virtual void destroy (object_t object)
    {
        delete (T*) object;
    }
};
};

#endif
