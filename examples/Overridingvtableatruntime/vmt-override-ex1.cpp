#include <iostream>
#include <typeinfo>
#include <stdexcept>

using namespace std;


// BaseClass - Defined as a base class to demonstrate our ability to replace 
//         vtable at runtime.
class BaseClass
{
    public:
        BaseClass()
        {
            cout << "BaseClass::BaseClass" << endl;
        }

        virtual void print()
        {
            cout << "BaseClass::print" << endl;
        }

        virtual  ~BaseClass()
        {
            cout << "BaseClass::~BaseClass" << endl;
        }
    
};


// DerivedClass - Defined as a derived class to demonstrate our ability to replace the vtable 
// for the class. For our example we will only support single inheritance, but multiple inheritance
// can also be implemented with a lot more effort.

class DerivedClass : public BaseClass
{
    public:
        virtual void print() 
        {
            cout << "DerivedClass::print" << endl;
        }

        ~DerivedClass()
        {
            cout << "DerivedClass::~DerivedClass" << endl;
        }
};


// DerivedAgain - We will be replacing the vtable of an instance of DerivedClass with
// the vtable from DerivedAgain class at runtime. As a general rule for this exercise
// whenever we are replacing the vtable of an instance of another class, the class
// which will carry the methods replacing the original methods MUST NOT have any data members,
// but all the base classes are allowed to have data members that may be accessed from the 
// class used to override vtable partially or completely. There are mechanisms that can still
// be used to store data while attaching it to the instance of the class used for overriding
// the vtable at runtime with proper construction, that we will discuss later on in this 
// article.

class DerivedAgain : public DerivedClass
{
public:

        virtual void print()  __attribute__((used))
        {
            cout << "DerivedAgain::print" << endl;
        }

        virtual ~DerivedAgain() __attribute__((used))
        {
            cout << "DerivedAgain::~DerivedAgain" << endl;
        }

};


// ReplaceVtable:
//      prerequisites: 
//          - oldT must be the pointer to an instance of the class with a vtable whose vtable is to be replaced.
//          - newT must be the class with a vtable whose vtable will be used to replace the vtable in oldT.
//          - newT must be a class derived from the original class of oldT or the original class itself.

#define ReplaceVtable(oldT, classNameLen, newT) {\
    void ** it = (void**)&(_ZTV ##classNameLen ## newT);\
    if (*it!=NULL)\
        throw std::logic_error("vtable layout does not match expectation: first pointer does not point to null");\
    it++;\
    if (&typeid(newT)!=*it)\
        throw std::logic_error("vtable layout does not match expectation: second pointer does not point to typeid");\
    it++;\
    *((void**)oldT) = it;\
    if (it==(void**)~0x0l)\
        ((newT*)oldT)->~newT();}


// ExternVtable is used to import a reference to the vtable so it may be used from the program.
#define ExternVtable(classNameLen, newT) extern "C" void* _ZTV ##classNameLen ## newT


ExternVtable(12, DerivedClass);
ExternVtable(12, DerivedAgain);
ExternVtable(9, BaseClass);

int main(int argc, char ** argv)
{
    DerivedClass* c = new DerivedClass();


    c->print();

    ReplaceVtable(c, 12, DerivedAgain);

    c->print();

    ReplaceVtable(c, 12, DerivedClass);

    c->print();

    ReplaceVtable(c, 9, BaseClass);

    c->print();

    ReplaceVtable(c, 12, DerivedClass);

    c->print();

    delete c;

    return 0;
}


