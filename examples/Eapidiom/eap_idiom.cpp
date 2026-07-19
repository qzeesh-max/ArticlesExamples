#include <iostream>
#include <functional>
#include <memory>

using namespace std;

template <typename T>
class ProtectedPointer
{
public:
	class PointerWrap
	{
		friend class ProtectedPointer<T>;
	private:
		const shared_ptr<T>& pointer_;
		const std::function<void (bool, const shared_ptr<T>&)>& functor_;

	private:
		PointerWrap& operator=(const PointerWrap&) = delete;

	protected:
		PointerWrap(const PointerWrap&) = default;
		PointerWrap(PointerWrap&&) = default;
	public:
		template<typename Functor>
		PointerWrap(const shared_ptr<T>& pointer, const Functor& functor) : pointer_(pointer), functor_(functor)
		{
			functor_(true, pointer_);
		}

		~PointerWrap()
		{
			functor_(false, pointer_);
		}

		T* operator->()
		{
			return pointer_.get();
		}

		const T* operator->() const
		{
			return pointer_.get();
		}
	};
private:
	shared_ptr<T> pointer_;
	std::function<void(bool, const shared_ptr<T>&)> functor_;
public:
	template<typename Functor>
	ProtectedPointer(T* pointer, const Functor& functor): pointer_(pointer), functor_(functor)
	{
	}

	PointerWrap operator->()
	{
		return PointerWrap(pointer_, functor_);
	}

	const PointerWrap operator->() const
	{
		return PointerWrap(pointer_, functor_);
	}
};

struct Vector{
	int x{0}, y{0}, z{0};

	void assign(int x_, int y_, int z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}
};


int main(void)
{
	ProtectedPointer<Vector> p{ new Vector, [](bool w, const shared_ptr<Vector>& s){
		cout << (w ? "before": "after") <<
			" x = " << s-> x <<
			" y = " << s-> y <<
			" z = " << s->z 
			<< endl;
	}};

	p->x=13;
	p->y=136;
	p->assign(23, 24, 19);
	p->z++;

	p.operator->()->x+=12;

	auto q(p);

	q->z++;

	return  0;
}
