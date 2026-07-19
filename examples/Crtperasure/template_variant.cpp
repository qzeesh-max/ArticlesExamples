#include<iostream>
#include<type_traits>
#include<typeindex>
#include<utility>
#include<algorithm>
#include<array>
#include<list>

template<typename...TYPES> struct ListTypes
{
    template<template <typename...> class TARGET>
    struct ApplyAll
    {
        using RESULT = TARGET<TYPES...>;
    };
};

template<typename CONCRETE, typename VISITOR, typename HINT>
struct VisitorBinding
{
    static void bound(void * v, void * o, const void * hint)
    {
        reinterpret_cast<VISITOR*>(v)->visit_templated_variant(*reinterpret_cast<const HINT*>(hint), *reinterpret_cast<CONCRETE*>(o));
    };
};

template <template <typename> class TEMPL, class T> 
struct HasSameBase
{
	template <typename U>
	static std::true_type TestType(const TEMPL<U>*);

	static std::false_type TestType(...);

	enum { value = std::is_same<decltype(TestType(std::declval<T*>())), std::true_type>::value };
};



template <template <typename> class TEMPL, class HINT, typename ALLOWED_VISITORS = ListTypes<>> class TemplatedVariant
{

    using TypeVisitorAssoc  = std::pair<std::type_index, void(*)(void*,void*,const void*)>;
private:
    void (*deleter_)(void*) {
        nullptr
    };
    void *state_ { nullptr };
    void (*visitor_)(void*, const TemplatedVariant<TEMPL, HINT, ALLOWED_VISITORS>*, const std::type_index&, void*, const HINT&) {
        nullptr
    };

    template<typename T>
    struct BindSpecialization
    {
        template <typename...V> struct Bind
        {
            static auto& value()
            {
                static std::array<TypeVisitorAssoc, sizeof...(V)> val = { TypeVisitorAssoc(typeid(V), &VisitorBinding<T, V, HINT>::bound)...};

                return val;
            }
        };
    };

public:
    TemplatedVariant(const TemplatedVariant&) = delete;
    TemplatedVariant(TemplatedVariant&&) = delete;
    TemplatedVariant& operator=(TemplatedVariant&&) = delete;
    TemplatedVariant& operator=(const TemplatedVariant&) = delete;

    TemplatedVariant()
    {
    }

    template <typename T>
    TemplatedVariant(const T& value)
    {
        store(value);
    }

    ~TemplatedVariant()
    {
        if (state_)
        {
            deleter_(state_);
        }
    }

    void reset()
    {
        if (state_)
        {
            deleter_(state_);
            state_ = nullptr;
        }
    }


    template <typename T>
    TemplatedVariant& operator=(const T& value)
    {
        store(value);

        return *this;
    }

    template <typename T>
    void store(const T& value_)
    {
	static_assert(HasSameBase<TEMPL, T>::value, "Require the same CRTP base for the type passed in.");

        auto oldState = state_;
        auto oldDeleter = deleter_;


        struct state
        {
            T stored_;

            static void visitor(void* self_, const TemplatedVariant<TEMPL, HINT, ALLOWED_VISITORS>* other_, const std::type_index& ti, void* v, const HINT& hint)
            {
                typedef typename ALLOWED_VISITORS::template ApplyAll<BindSpecialization<T>::template Bind>::RESULT RESULT;
                static auto associations = RESULT::value();
                state* self = reinterpret_cast<state*>(self_);

                for (auto& vi: associations)
                {
                    if (vi.first == ti)
                    {
                        vi.second(v, &self->stored_, &hint);
                        return;
                    }

                }
                other_->template fail_visit<>(&self->stored_);
            }
        };

        state_ = new state{value_};

        if (oldState!=nullptr)
        {
            oldDeleter(oldState);
        }

        deleter_ = [](void * p) {
            delete reinterpret_cast<state*>(p);
        };
        visitor_ = &state::visitor;
    }

    bool empty() const
    {
        return state_!=nullptr;
    }

    template<typename FUNCTOR>
    void accept(const HINT& hint, FUNCTOR &&f) const
    {
        if (state_!=nullptr)
        {
            visitor_(state_, this, typeid(f), &f, hint);
        } else {
            throw std::logic_error("Empty TemplVariant cannot be visited.");
        }
    }
private:
    template <typename T>
    void fail_visit(TEMPL<T>* v) const
    {
        throw std::logic_error("Unregistered visitor type");
    }

};

template<typename T> struct ArbStore
{
    T value_;
};


class SampleClass
{
private:
    struct PrintFunctor1
    {
        template <typename T>
        void visit_templated_variant(int hint, ArbStore<T>& o)
        {
            std::cout << hint << ": First Visitor = " << o.value_ << std::endl;
        }
    };

    struct PrintFunctor2
    {
        template <typename T>
        void visit_templated_variant(int hint, ArbStore<T>& o)
        {
            std::cout << hint << ": Second Visitor = " << o.value_ << std::endl;
        }

    };

protected:
    using UsedTypes = ListTypes<PrintFunctor1, PrintFunctor2>;
    using StoreType =  TemplatedVariant<ArbStore, int, UsedTypes>;

    std::list<StoreType>  data_;

public:
    template <typename T>
    void Set(const ArbStore<T>& v)
    {
        data_.emplace_back(v);
    }

    void Visit1(int hint)
    {
        for (auto& data : data_)
        {
            data.accept(hint, PrintFunctor1());
        }
    }

    void Visit2(int hint)
    {
        for (auto& data : data_)
        {
            data.accept(hint, PrintFunctor2());
        }
    }

    void QuickTest(const StoreType& w)
    {
        w.accept(sizeof(w), PrintFunctor2());
    }
};

template <typename T>
struct ArbStore2 : public ArbStore<T>
{
	using BASE = ArbStore<T>;
	ArbStore2(T a) : BASE{a}
	{
	}
};

int main()
{
    SampleClass var;

    var.Set(ArbStore<int> {10});
    var.Visit1(1);
    var.Visit2(2);
    var.Set(ArbStore<const char*> {"awesome"});
    var.Visit1(3);

    var.QuickTest(ArbStore<bool> {false});
    var.QuickTest(ArbStore2<const char *>("arb-store-2"));

    return 0;
};
