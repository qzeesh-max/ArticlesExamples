#ifndef ORDERED_STATIC_INITIALIZER_HPP
#define ORDERED_STATIC_INITIALIZER_HPP

#include <string>
#include <set>
#include <deque>
#include <stdint.h>
#include <stdexcept>


/*
	OrderedStatic - Base class for TypedOrderedStatic<T> used to provide a mechanism for evaluating dependencies by OrderedStaticInitializer class
*/
class OrderedStatic
{
	protected:
		std::set<OrderedStatic*> requiredStatics;

	protected:

		// Add a dependency for this static
		void RequireStatic(OrderedStatic* static_)
		{
			requiredStatics.insert(static_);
		}

	public:
		// Check to see if set_ contains all the dependencies for this static
		bool FindAllStatics(const std::set<OrderedStatic*>& set_)
		{
			for (OrderedStatic* static_ : requiredStatics)
			{
				if (set_.count(static_)==0)
					return false;
			}

			return true;
		}

		// Constructor that creates the base with the destructor for the object 
		OrderedStatic(void (*f)(void*)) : Destroy(f), Header(Signature)
		{
		}

		// Virtual method for initialization of the static when all the dependencies are ready
		virtual void Init() = 0;

		// Member variable that points to the function that would be used to destroy the static. This is a member function
		// in the TypedOrderedStatic<T>
		void (*Destroy)(void*);


	protected:
		// Signature for class to determine if the object was created in global section or not
		static constexpr uint32_t Signature = 0x01234567;

		// Position where the class looks for a signature to tell if it was initialized or not
		uint32_t Header;

	public:
		// Returns true if the object is created
		bool Inited() const 
		{
			return Header==Signature;
		}

		// Returns the static variables this static depends on
		std::set<OrderedStatic*>& GetRequiredStatics()
		{
			return requiredStatics;
		}
};

// class OrderedStaticInitializer is used for creation of statics in an orderly fashion
// via macros in this header file at the very end. The class is designed so the functions
// in it may be called before it is actually created so that unorderly call via static
// initializer in C++ runtime does not cause chaos
class OrderedStaticInitializer
{
	private:
		// a template class to allocate space for another object without actually initializing it unless it is needed,
		// sort of like boost optional except there is no constructor that writes anything, because we do not want second
		// call to constructor to pollute the memory used by this object
		template <typename T>
		class CreateAsNeeded
		{
			private:
				uint32_t data[sizeof(T) / sizeof(uint32_t) +
					      ((sizeof(T) % sizeof(uint32_t)) ? 1 : 0)];

			public:
				CreateAsNeeded()
				{
				}

				template <typename... U>
				void Construct(U... value)
				{
					new (data) T (value...);
				}

				void Destroy()
				{
					reinterpret_cast<T*>(data)->~T();
				}

				T* operator->()
				{
					return reinterpret_cast<T*>(data);
				}

				T& operator*()
				{
					return *reinterpret_cast<T*>(data);
				}
		};
	private:
		// Signatures in the memory of this object that allows the object to determine if it has been created
		// yet or not. The functions in this object that are exposed to public all cause the object to be created
		// via the ConstructIfNeeded method if it does not exist yet.
		uint32_t initializationSignature0, initializationSignature1;
		static constexpr uint32_t expectedSignature0 = 0x01234567;
		static constexpr uint32_t expectedSignature1 = 0x76543210;
		
		// Members used to keep track of statics that are either initialized or
		// are in the process of being initialized
		CreateAsNeeded<std::set<OrderedStatic*> > initializedStaticsSet;
		CreateAsNeeded<std::deque<OrderedStatic*> > initializedStatics;
		CreateAsNeeded<std::set<OrderedStatic*> > pendingInitStatics;
	private:
		// Method that causes the object to be created if it does not already
		// exist
		void ConstructIfNeeded()
		{
			if (initializationSignature0!=expectedSignature0 &&
			    initializationSignature1!=expectedSignature1)
			{
				initializedStaticsSet.Construct();
				initializedStatics.Construct();
				pendingInitStatics.Construct();

				initializationSignature0 = expectedSignature0;
				initializationSignature1 = expectedSignature1;
			}
		}
	public:
		OrderedStaticInitializer()
		{
			ConstructIfNeeded();
		}

		// Destructor causes all the statics managed by this object to be destroyed
		// in reverse order of creation
		~OrderedStaticInitializer()
		{
			for (OrderedStatic * static_ : *initializedStatics)
				static_->Destroy(static_);

			initializedStaticsSet.Destroy();
			initializedStatics.Destroy();
			pendingInitStatics.Destroy();
		}		

		// Method that is used to add a static that has already been initialized and to cause
		// all other statics that depend on it to be initialized as well
		void AddInitializedStatic(OrderedStatic* static_)
		{
			static int recursion = 0;
			static int repeat = 0;

			++recursion;


			ConstructIfNeeded();


			if (initializedStaticsSet->count(static_)==0)
			{
				initializedStaticsSet->insert(static_);
				initializedStatics->push_back(static_);
			}

			if (recursion > 1)
			{
				--recursion;
				++repeat;
				return;
			}

			do
			{
				for (auto pendingIt = pendingInitStatics->begin(), pendingEnd = pendingInitStatics->end(), nextIt = pendingIt;
					pendingIt != pendingEnd; pendingIt = nextIt)
				{
					nextIt++;
		
					auto pending_ = *pendingIt;

					if (pending_->Inited() && pending_->FindAllStatics(*initializedStaticsSet))
					{
						pendingInitStatics->erase(pendingIt);	
						pending_->Init();
					}
				}
				if (repeat > 0)
				{
					--repeat;
					continue;
				} 
				break;
			} while (true);



			--recursion;
		}

		// Checks if there are any circular dependencies between statics and throw an exception if one is found
		void ValidateCircular()
		{
			ConstructIfNeeded();			
			for (auto outer : *pendingInitStatics)
			{
				if (outer->Inited())
				{
					for (auto inner: outer->GetRequiredStatics())
					{
						if (outer==inner)
							throw std::logic_error("Circular dependency in Ordered Static Initialization");

						std::set<OrderedStatic*> others;

						others.insert(outer);

						if (inner->Inited())
							Recurse(outer, inner, others);
					}
				}
			}

		}


	private:
		// Recurses through all the static dependencies looking for circular dependencies.
		void Recurse(OrderedStatic* outer, OrderedStatic * inner, std::set<OrderedStatic*>& others)
		{
			others.insert(inner);

			for (auto innerInner : inner->GetRequiredStatics())
			{

				if (others.count(innerInner))
					throw std::logic_error("Circular dependency in Ordered Static Initialization");

						
	
				if (innerInner->Inited())
				{
					std::set<OrderedStatic*> nextOthers = others;


					Recurse(outer, innerInner, nextOthers);
				}
			}
		}

	public:
		// Returns true if the specific static has been initialized already
		bool HaveStatic(OrderedStatic* static_)
		{
			ConstructIfNeeded();

			return initializedStaticsSet->count(static_)!=0;
		}

		// Adds a static that has not been initialized because one of its dependency is not there yet.
		void AddPendingStatic(OrderedStatic* static_)
		{
			ConstructIfNeeded();
			pendingInitStatics->insert(static_);
		}

		
};

// global function used by the macros to access an instance of OrderedStaticInitializer
extern OrderedStaticInitializer& GetStaticInitializer();

// TypedOrderedStatic initializer that tracks initialized and uninitialized static along with their dependencies.
template <typename T>
class TypedOrderedStatic : public OrderedStatic
{
	public:
		typedef void (*InitFunction)(void*);

	protected:
		uint32_t value_[sizeof(T)/sizeof(uint32_t) + ((sizeof(T)%sizeof(uint32_t)) ? 1 : 0)];
		InitFunction InitFunc;
	public:
		TypedOrderedStatic() : InitFunc(nullptr), OrderedStatic(&_Destroy)
		{

		}

		// constructs the static
		void Init()
		{
			if (InitFunc!=nullptr)
			{
				InitFunc(value_);
			}

			GetStaticInitializer().AddInitializedStatic(this);
		}



	private:
		// destroys the static
		static void _Destroy(void* _void_this)
		{
			TypedOrderedStatic<T>* _this = static_cast<TypedOrderedStatic<T>*>(_void_this);

			if (_this->InitFunc!=nullptr)
				reinterpret_cast<T*>(_this->value_)->~T();
		}

	public:
		// a structure that causes the construction of the static only to occur after assignment
		// of the reference returned from it, otherwise the reference will stay null
		struct deferred_init_return
		{
			private:
				TypedOrderedStatic<T>* orderedStatic;
				void* 		     value;
				bool		     valueRetrieved;
				bool		     callInit;

			public:
				deferred_init_return(TypedOrderedStatic<T>* orderedStatic, void *value) : 
					orderedStatic(orderedStatic), value(value), valueRetrieved(false)
				{
				}

				~deferred_init_return()
				{
					if (valueRetrieved)
						orderedStatic->Construct();
				}			

				// proxy for the Requires method in TypedOrderedStatic<T>
				deferred_init_return Requires(OrderedStatic& other)
				{
					return orderedStatic->Requires(other);			
				}

				// returns a reference to the static, and signal that the constructor should get called
				operator T&()
				{
					valueRetrieved = true;

					return *reinterpret_cast<T*>(value);

				}
		};


		// causes the constructor call to be recorded and be invoked when the reference is ready
		deferred_init_return Value(InitFunction initFunc)
		{
			InitFunc = initFunc;

			return deferred_init_return(this, value_);
		}


		// registers a dependecy for this static, unless it has already been constructed
		deferred_init_return Requires(OrderedStatic& other)
		{
			if (!GetStaticInitializer().HaveStatic(&other))
				RequireStatic(&other);

			return deferred_init_return(this, value_);
		}

		friend class deferred_init_return;

	protected:
		// constructs the object
		void Construct()
		{
			if (requiredStatics.size()==0 && InitFunc)
				Init();	
			else if (requiredStatics.size()!=0)
			{
				GetStaticInitializer().AddPendingStatic(this);
				GetStaticInitializer().ValidateCircular();
			}
		}
};


// Define a non-member global scope static. Constructor parameters are optionally placed in the third argument, which may otherwise be left out.
#define ORDERED_NONMEM_STATIC(type, name, ...)\
	TypedOrderedStatic<type> ORDERED_NOMEM_ ## name; type& name = \
 ORDERED_NOMEM_ ## name.Value([](void * v) { new (v) type\
( __VA_ARGS__); })	

// Declares a dependency on a non-member global scope static
#define STATIC_REQUIRES_NONMEM(name) .Requires(ORDERED_NOMEM_ ## name)

// Declares a non-member global scope static
#define ORDERED_NONMEM_STATIC_DECL(type, name) extern type& name; extern TypedOrderedStatic<type> ORDERED_NOMEM_ ## name;

// Define a class scope static
#define ORDERED_MEM_STATIC(type, class, name, ...)\
	TypedOrderedStatic<type> class::ORDERED_MEM_ ## class ## _ ## name; type& class::name = \
 ORDERED_MEM_ ## class ## _ ## name.Value([](void * v) { new (v) type\
( __VA_ARGS__); })	

// Declares a class scope static is a dependency
#define STATIC_REQUIRES_MEM(class, name) .Requires(class::ORDERED_MEM_ ## class ## _ ## name)

// Declares a class scope static (inside a class or struct body)
#define ORDERED_MEM_STATIC_DECL(type, class, name) static type& name; static TypedOrderedStatic<type> ORDERED_MEM_ ## class ## _ ## name;



#endif
