#ifndef _REFLECTION_CALL
#define _REFLECTION_CALL

#pragma once

namespace reflection
{
// Entrypoint template for Call argument stacking structure that takes arguments from a list and stacks them as call arguments, 
// while performing type, const-correctness and reference-correctness checking on all of them. Actual call to the method is made
// by the terminating template for Call argument stacking.
template < int count, class class_t, typename ret_t, typename ... arg_t > struct call
{

// Calls for const methods

    call (object_t object, ret_t (class_t::*method) (arg_t ...) const, 
	  boost::any& returnValue, 
	  ParameterDefinitions_t::const_iterator& pit, 
	  ParameterNames_t::const_iterator& nit, 
          Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        if ((*it).can_pass_as_argument (*pit))
        {
            Parameters_t::iterator vit = it;
            call < count - 1, class_t, ret_t, arg_t ... > (object, method, returnValue, ++pit, ++nit, ++it, end, *vit);
        }
        else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
    }

    template < typename ... T > call (object_t object, ret_t (class_t::*method) (arg_t ...) const, 
	 			      ParameterDefinitions_t::const_iterator& pit, 
				      ParameterNames_t::const_iterator& nit, 
                                      Parameters_t::iterator& it, Parameters_t::iterator& end, T ... args)
    {
        if ((*it).can_pass_as_argument (*pit))
        {
            Parameters_t::iterator vit = it;
            call < count - 1, class_t, ret_t, arg_t ... > (object, method, ++pit, ++nit, ++it, end, args ..., *vit);
        }
        else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
    }

    call (object_t object, ret_t (class_t::*method) (arg_t ...) const, 
          ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
          Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        if ((*it).can_pass_as_argument (*pit))
        {
            Parameters_t::iterator vit = it;
            call < count - 1, class_t, ret_t, arg_t ... > (object, method, ++pit, ++nit, ++it, end, *vit);
        }
        else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
    }

    template < typename ... T > call (object_t object, ret_t (class_t::*method) (arg_t ...) const, 
				      boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, 
                                      ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, 
				      Parameters_t::iterator& end, T ... args)
    {
        if ((*it).can_pass_as_argument (*pit))
        {
            Parameters_t::iterator vit = it;
            call < count - 1, class_t, ret_t, arg_t ... > (object, method, returnValue, ++pit, ++nit, ++it, end, args ..., *vit);
        }
        else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
    }

// Calls for non-const methods

    call (object_t object, ret_t (class_t::*method) (arg_t ...), 
	  boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, 
          ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        if ((*it).can_pass_as_argument (*pit))
        {
            Parameters_t::iterator vit = it;
            call < count - 1, class_t, ret_t, arg_t ... > (object, method, returnValue, ++pit, ++nit, ++it, end, *vit);
        }
        else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
    }

    template < typename ... T > call (object_t object, ret_t (class_t::*method) (arg_t ...), 
				      ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
				      Parameters_t::iterator& it, Parameters_t::iterator& end, T ... args)
    {
        if ((*it).can_pass_as_argument (*pit))
        {
            Parameters_t::iterator vit = it;
            call < count - 1, class_t, ret_t, arg_t ... > (object, method, ++pit, ++nit, ++it, end, args ..., *vit);
        }
        else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
    }

    call (object_t object, ret_t (class_t::*method) (arg_t ...), 
	  ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
          Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        if ((*it).can_pass_as_argument (*pit))
        {
            Parameters_t::iterator vit = it;
            call < count - 1, class_t, ret_t, arg_t ... > (object, method, ++pit, ++nit, ++it, end, *vit);
        }
        else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
    }

    template < typename ... T > call (object_t object, ret_t (class_t::*method) (arg_t ...), 
				      boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, 
				      ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, 
				      Parameters_t::iterator& end, T ... args)
    {
        if ((*it).can_pass_as_argument (*pit))
        {
            Parameters_t::iterator vit = it;
            call < count - 1, class_t, ret_t, arg_t ... > (object, method, returnValue, ++pit, ++nit, ++it, end, args ..., *vit);
        }
        else throw logic_error (("Typemismatch for argument:" + *nit).c_str ());
    }

};

// Terminating template for Call argument stacking structure. This struture makes the actual call on the method for methods with
// a return type.
template < typename class_t, typename ret_t, typename ... arg_t > struct call < 0, class_t, ret_t, arg_t ... >
{

// Call for const methods

    call (object_t object, ret_t (class_t::*method) (arg_t ...) const, 
	  boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, 
	  ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        returnValue = (((class_t*) object)->*method) ();
    }

    template < typename ... T > call (object_t object, ret_t (class_t::*method) (arg_t ...) const, 
				      ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
                                      Parameters_t::iterator& it, Parameters_t::iterator& end, T ... args)
    {
        (((class_t*) object)->*method) (args...);
    }

    call (object_t object, ret_t (class_t::*method) (arg_t ...) const, 
	  ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
          Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        (((class_t*) object)->*method) ();
    }

    template < typename ... T > call (object_t object, ret_t (class_t::*method) (arg_t ...) const, 
				      boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, 
				      ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, 
                                      Parameters_t::iterator& end, T ... args)
    {
        returnValue = (((class_t*) object)->*method) (args...);
    }

// Call for non-const methods

    call (object_t object, ret_t (class_t::*method) (arg_t ...), 
	  boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, 
	  ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        returnValue = (((class_t*) object)->*method) ();
    }

    template < typename ... T > call (object_t object, ret_t (class_t::*method) (arg_t ...), ParameterDefinitions_t::const_iterator& pit, 
				      ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, Parameters_t::iterator& end, T ... args)
    {
        (((class_t*) object)->*method) (args...);
    }

    call (object_t object, ret_t (class_t::*method) (arg_t ...) , ParameterDefinitions_t::const_iterator& pit, 
	  ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        (((class_t*) object)->*method) ();
    }

    template < typename ... T > call (object_t object, ret_t (class_t::*method) (arg_t ...), 
				      boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
				      Parameters_t::iterator& it, Parameters_t::iterator& end, T ... args)
    {
        returnValue = (((class_t*) object)->*method) (args...);
    }
};


// Terminating template for Call argument stacking structure. This struture makes the actual call on the method for methods without
// a return type. You will notice that there are some calls in here with returnValue, but those are only for completeness, and do not
// actually result in code being generated. The calls with return types are referenced from within virtual functions.
template < typename class_t, typename ... arg_t > struct call < 0, class_t, void, arg_t ... >
{
// calls for const methods
    call (object_t object, void (class_t::*method) (arg_t ...) const, 
	  boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, 
	  ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, 
          Parameters_t::iterator& end)
    {
        returnValue = (((class_t*) object)->*method) ();
    }

    template < typename ... T > call (object_t object, void (class_t::*method) (arg_t ...) const, 
				      ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
				      Parameters_t::iterator& it, Parameters_t::iterator& end, T ... args)
    {
        (((class_t*) object)->*method) (args...);
    }

    call (object_t object, void (class_t::*method) (arg_t ...) const, 
	  ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
          Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        (((class_t*) object)->*method) ();
    }

    template < typename ... T > call (object_t object, void (class_t::*method) (arg_t ...) const, 
				      boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, 
				      ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, 
				      Parameters_t::iterator& end, T ... args)
    {
        (((class_t*) object)->*method) (args...);
    }

// Call for non-const methods

    call (object_t object, void (class_t::*method) (arg_t ...) , boost::any& returnValue, 
	  ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
          Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        (((class_t*) object)->*method) ();
    }

    template < typename ... T > call (object_t object, void (class_t::*method) (arg_t ...) , 
				      ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
			              Parameters_t::iterator& it, Parameters_t::iterator& end, T ... args)
    {
        (((class_t*) object)->*method) (args...);
    }

    call (object_t object, void (class_t::*method) (arg_t ...) , 
	  ParameterDefinitions_t::const_iterator& pit, ParameterNames_t::const_iterator& nit, 
          Parameters_t::iterator& it, Parameters_t::iterator& end)
    {
        (((class_t*) object)->*method) ();
    }

    template < typename ... T > call (object_t object, void (class_t::*method) (arg_t ...) , 
				      boost::any& returnValue, ParameterDefinitions_t::const_iterator& pit, 
				      ParameterNames_t::const_iterator& nit, Parameters_t::iterator& it, 
				      Parameters_t::iterator& end, T ... args)
    {
        (((class_t*) object)->*method) (args...);
    }
};
}

#endif
