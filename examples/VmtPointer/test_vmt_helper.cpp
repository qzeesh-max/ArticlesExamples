#include <cassert>
#include <iostream>
#include <stdint.h>
#include <typeinfo>
#include "virtual_method_helper.hpp"



using namespace std;
using namespace virtual_method_helper;

struct AnotherBase
{
	int i;
	virtual ~AnotherBase()
	{
		cout << "DestroyAnotherBase" << std::endl;
	}
	virtual void Carrot()
	{
		cout << "Carrot" << endl;
	}
};
struct Base
{
	int j;
	virtual void Test()
	{
		cout << "Base" << endl;
	}

	virtual void SecondTest()
	{
		cout << "SecondTest:Base" << endl;
	}

	virtual ~Base()
	{
		cout << "DestroyBase" << std::endl;
	}
};

struct Derived : public AnotherBase, public Base
{
	int k;
	virtual void Test()
	{
		cout << "Derived" << endl;
	}

	virtual void SecondTest()
	{
		cout << "SecondTest:Derived" << endl;
	}


	~Derived()
	{
		cout << "DestroyDerived" << endl;
	}
};

struct DerivedNext : public AnotherBase, public Base
{
	int l;
	~DerivedNext()
	{
		cout << "DestroyDerivedNext" << endl;
	}

	virtual void SecondTest()
	{
		cout << "SecondTest:DerivedNext" << endl;
	}

};

struct DerivedNextNext : public DerivedNext
{
	int m;

	~DerivedNextNext()
	{
		cout << "DestroyDerivedNextNext" << endl;
	}

	virtual void Test()
	{
		cout << "Yo man!" << endl;
	}
};




int main(void)
{
	Base b;
	Derived d;
	DerivedNext q;
	DerivedNextNext dnn;

	cout << "Asserting that all derived pointers are detected." << std::endl;

	assert(getResolvedPointer(&b, &Base::Test) != getResolvedPointer(&d, &Base::Test));
	assert(getResolvedPointer(&b, &Base::Test) == getResolvedPointer(&q, &Base::Test));
	assert(getResolvedPointer(&b, &Base::Test) != getResolvedPointer(&dnn, &Base::Test));
	assert(getResolvedPointer(&b, &Base::SecondTest) != getResolvedPointer(&d, &Base::SecondTest));
	assert(getResolvedPointer(&b, &Base::SecondTest) != getResolvedPointer(&q, &Base::SecondTest));
	assert(getResolvedPointer(&b, &Base::SecondTest) != getResolvedPointer(&dnn, &Base::SecondTest));

	cout << endl;
	cout << "Calling normal way" << endl;

	b.Test();
	d.Test();
	q.Test();
	dnn.Test();
	b.SecondTest();
	d.SecondTest();
	q.SecondTest();
	dnn.SecondTest();

	cout << endl;
	cout << "Calling using resolved pointers" << endl;

	getResolvedPointer(&b, &Base::Test)(&b);
	getResolvedPointer(&d, &Base::Test)(&d);
	getResolvedPointer(&q, &Base::Test)(&q);
	getResolvedPointer(&dnn, &Base::Test)(&dnn);
	getResolvedPointer(&b, &Base::SecondTest)(&b);
	getResolvedPointer(&d, &Base::SecondTest)(&d);
	getResolvedPointer(&q, &Base::SecondTest)(&q);
	getResolvedPointer(&dnn, &Base::SecondTest)(&dnn);

	cout << endl;
	cout << "Terminating..." << endl;

	return 0;
}
