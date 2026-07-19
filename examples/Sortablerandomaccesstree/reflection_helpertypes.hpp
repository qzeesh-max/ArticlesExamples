#ifndef _REFLECTION_HELPERTYPES
#define _REFLECTION_HELPERTYPES

#pragma once

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <typeinfo>
#include <list>
#include <vector>
#include <string>
#include <stdexcept>

namespace reflection
{
using namespace std;

class runtime_type_details_t
{
	protected:
		type_info const* _type;
		bool	  	 _const;
		bool		 _reference;

		template <typename T> struct type_holder {};

		template <typename T=void>
		runtime_type_details_t(type_holder<T>):
			_type(&typeid(T)),
			_const(boost::is_const<T>::value),
			_reference(boost::is_reference<T>::value)
		{
		}
	

		

	public:
		runtime_type_details_t() : runtime_type_details_t(type_holder<void>())
		{
		}

		template <typename T>
		static runtime_type_details_t of()
		{
			return runtime_type_details_t(type_holder<T>());
		}


		type_info const & getTypeID() const
		{
			return *_type;
		}

		bool isConst() const
		{
			return _const;
		}


		bool isReference() const
		{
			return _reference;
		}


		bool operator==(const runtime_type_details_t& other) const
		{
			return (*_type==*other._type) && (_const==other._const) && (_reference==other._reference);
		}


		bool operator!=(const runtime_type_details_t& other) const
		{
			return (*_type!=*other._type) || (_const!=other._const) || (_reference!=other._reference);
		}


		bool operator<(const runtime_type_details_t& other) const
		{
			return (_type->before(*other._type) || 
			       ((*_type==*other._type) && ((_const < other._const) || ((_const == other._const) && (_reference < other._reference)))));
		}
};

class typed_reference_t
{
private:

// holder and specialholder class will provide mechanism for holding copy of values for pass by value arguments.
    class holder
    {
    public:
        virtual ~holder()
        {
        }

        virtual void* value() = 0;
        virtual holder* clone() = 0;
    };

    template<typename T>
    class specialholder : public holder
    {
    private:
        T _value;

    public:
        specialholder(T _value) : _value(_value)
        {
        }

        void * value() {
            return &_value;
        }

        virtual holder* clone() {
            return new specialholder<T>(_value);
        }
    };
private:
    runtime_type_details_t _type;
    void* _value;
    boost::shared_ptr<holder> _holder;







public:
    typed_reference_t() :
        _type(),
        _value(NULL),
        _holder()
    {
    }

    typed_reference_t(const typed_reference_t& other):
        _type(other._type),
        _value(other._value),
        _holder(other._holder)

    {
    }

    typed_reference_t(typed_reference_t& other):
        _type(other._type),
        _value(other._value),
        _holder(other._holder)

    {
    }

// we provide a reference_wrapper based constructor for passing references without running into
// ambiguity when passing primitive / value type literals.
    template <typename T>
    typed_reference_t(const boost::reference_wrapper<T>& _value) :
        _type(runtime_type_details_t::of<T&>()),
        _value((void*)&boost::unwrap_ref(_value)),
        _holder()
    {
    }


    template <typename T>
    typed_reference_t(T _value) :
        _type(runtime_type_details_t::of<T>()),
        _holder(new specialholder<T>(_value))
    {
        this->_value = _holder.get()->value();
    }

    ~typed_reference_t()
    {
    }



    runtime_type_details_t type() const
    {
	return _type;
    }
	

    bool can_pass_as_argument(const runtime_type_details_t& type)
    {
        // same type, and formal const -or-
        // same type, and both reference and both non-const
        return (type.getTypeID()==_type.getTypeID()) &&
	       ((type.isReference() && !type.isConst() && _type.isReference() && !_type.isConst()) ||
		((type.isReference() && type.isConst()) || (!type.isReference())));

    }

    template <typename T>
    operator T& ()
    {
	if (typeid(T)==_type.getTypeID())
	        return *(T*)_value;
	else
		throw std::logic_error("Type mismatch");
    }

    template <typename T>
    operator const T& ()
    {
	if (typeid(T)==_type.getTypeID())
	        return *(T*)_value;
	else
		throw std::logic_error("Type mismatch");
    }
};



typedef list<runtime_type_details_t>ParameterDefinitions_t;
typedef vector < string > ParameterNames_t;
typedef list <  typed_reference_t >   Parameters_t;
typedef typed_reference_t object_t;

}




#endif
