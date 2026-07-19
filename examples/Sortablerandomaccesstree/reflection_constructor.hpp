#ifndef _REFLECTION_CONSTRUCTOR
#define _REFLECTION_CONSTRUCTOR

namespace reflection
{


template <int count, typename class_t, typename...arg_t> 
struct construct;


template <int count, typename class_t, typename...arg_t> 
struct construct
{
	class_t* operator()(ParameterDefinitions_t::const_iterator& pit, 
	  						    ParameterNames_t::const_iterator& nit, 	
          						    Parameters_t::iterator& it, Parameters_t::iterator& end)
	{
		if ((*it).can_pass_as_argument (*pit))
        	{
        	    Parameters_t::iterator vit = it;
        	    return construct<count-1, class_t>()(++pit, ++nit, ++it, end, *vit);
        	}
        	else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
	}


	class_t* operator()(ParameterDefinitions_t::const_iterator& pit,	
			   ParameterNames_t::const_iterator& nit, 	
     			   Parameters_t::iterator& it, Parameters_t::iterator& end,
			   arg_t...arg)
	{
		if ((*it).can_pass_as_argument (*pit))
       		{
            		Parameters_t::iterator vit = it;
            		return construct<count-1, class_t, arg_t...>() (++pit, ++nit, ++it, end, arg..., *vit);
        	}
        	else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
	}
};

template <typename class_t, typename...arg_t> 
struct construct<0, class_t, arg_t...>
{
	class_t* operator()(ParameterDefinitions_t::const_iterator& pit, 
		           ParameterNames_t::const_iterator& nit, 	
          		   Parameters_t::iterator& it, Parameters_t::iterator& end,
			   arg_t...arg)
	{
	
        	return new class_t (arg...);
        
	}
};

class IConstructor
{
public:
	virtual object_t construct(Parameters_t& parameters) = 0;
};

template <typename class_t, typename...arg_t>
class Constructor : public IConstructor
{
protected:
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
	Constructor ()
   	{
        	addParameters < sizeof ... (arg_t), arg_t ... >();
        }

	
	virtual object_t construct(Parameters_t& parameters)
	{
	    Parameters_t::iterator it = parameters.begin (), end = parameters.end ();
            ParameterDefinitions_t::const_iterator pit = parameterDefinitions.begin ();
            ParameterNames_t::const_iterator nit = parameterNames.begin ();

	    return reflection::construct<sizeof...(arg_t), class_t>()(pit, nit, it, end);
	}
};

};


#endif
