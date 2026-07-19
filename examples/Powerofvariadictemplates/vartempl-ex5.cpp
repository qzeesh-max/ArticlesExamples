
#include <iostream>
#include <stdio.h>
#include <list>

using namespace std;

void print()
{
  std::cout.flush();
}

template <typename FirstArgumentType, typename ...ArgumentType>
void print(const FirstArgumentType& firstArgument, const ArgumentType&... arguments)
{
  std::cout << firstArgument;
  print(arguments...);
}

//This declares the base template with variadic arguments to tell the compiler
//that we will be providing it specializations
template<typename... ArgumentType>
class ParameterStoreImpl;


//This specialization allows invocation of the actual method and invocation of this
//specialization actually trickles down through recursive instantiation of the ParameterStoreImpl
//template.
template<>
class ParameterStoreImpl<>
{
public:
  template <typename ClassType, typename MethodType, typename... MethodArgumentType>
    void Execute(ClassType& instance, MethodType method, MethodArgumentType... arguments)
    {
      (instance.*method)(arguments...);
    }
};

//This specialization is designed to actually extract the first template argument
//from the variadic template arguments so that a field may be declared to store its value.
//The Execute method in this specialization actually concatenates the argument stored in this
//instance of ParameterStoreImpl specialization to the list of arguments passed from previous
//invocation of the Execute function in another specialization.
template <typename FirstType, typename... ArgumentType>
class ParameterStoreImpl<FirstType, ArgumentType...>
{
public:
  FirstType FirstValue;
  ParameterStoreImpl<ArgumentType...> MoreValues;

  ParameterStoreImpl(FirstType firstValue, ArgumentType... moreValues):
    FirstValue(firstValue), MoreValues(moreValues...)
  {
  }

  ~ParameterStoreImpl()
  {
  }

  template <typename ClassType, typename MethodType, typename... MethodArgumentType>
  void Execute(ClassType& instance, MethodType method, MethodArgumentType... arguments)
  {

    MoreValues.Execute(instance, method, arguments..., FirstValue);
  }
};

//Abstract class for invoking a bounded method stored by the Operation class.
class IOperation
{
public:  virtual void Execute() = 0;

  virtual ~IOperation() {}
};


//This class is used to bind parameters for a call to a method, and for invoking the method with those parameters.
template <typename ClassType, typename MethodType, typename... ArgumentType>
class Operation : public IOperation
{
protected:
  ClassType& Instance;
  MethodType Method;
  ParameterStoreImpl<ArgumentType...> Arguments;

public:
  Operation(ClassType& instance, 
    MethodType method, 
    ArgumentType&... arguments) :
    Instance(instance),
    Method(method),
    Arguments(arguments...)
  {
  }

  virtual ~Operation() {}

  virtual void Execute()
  {
    Arguments.Execute(Instance, Method);
  }
};


//This is the OperationList class to which one may queue up function invocations for later execution.
class OperationList
{
protected:
  typedef std::list<IOperation*> operations_t;
  operations_t Operations;

public:
  OperationList()
  {
  }

  virtual ~OperationList()
  {
    for (operations_t::iterator it = Operations.begin(),
      lastIt = Operations.end();
      it!=lastIt; it++)
    {
    delete *it;
    }
  }

template <typename ClassType, typename MethodType, typename... ArgumentType>
  void PushBack(ClassType& instance, MethodType method, ArgumentType... arguments)
  {
    Operations.push_back(new Operation<ClassType, MethodType, ArgumentType...>(instance, method, arguments...));
  }

  template <typename ClassType, typename MethodType, typename... ArgumentType>
  void PushFront(ClassType& instance, MethodType method, ArgumentType... arguments)
  {
    Operations.push_back(new Operation<ClassType, MethodType, ArgumentType...>(instance, method, arguments...));
  }


  void InvokeOperations()
  {
  for (operations_t::iterator it = Operations.begin(),
      lastIt = Operations.end();
      it!=lastIt;	it++)
    {
      (*it)->Execute();
    }
  }
};

class TestClass
{
public:
  void TestMethod1()
  {
    std::cout << "Sing a song of six pence," << endl;
    std::cout <<"a pocket full of rye," << endl;
  }

  void TestMethod2(int someNumber)
  {
    std::cout << "The number is " << someNumber << endl;
  }

  void TestMethod3(int someNumber, const string& someText)
  {
    std::cout << "someText: " << someText << endl;
    std::cout << "someNumber: " << someNumber << endl;
  }

  void PrintList(const std::list<std::string>* k)
  {
    std::cout << " list size = " << k->size() << std::endl;
  }
};


//Here is an example application to show you how to use the above classes.
int main()
{
  TestClass testObject;

  OperationList l;
  std::list<std::string> TestString;

  l.PushBack(testObject, &TestClass::TestMethod1);
  l.PushBack(testObject, &TestClass::TestMethod2, 12);
  l.PushFront(testObject, &TestClass::TestMethod3, 15, "Hello World");

  void (std::list<std::string>::*std_list_string_push_back)(const std::string&) = &std::list<std::string>::push_back;

  l.PushBack(TestString, std_list_string_push_back, "Test");
  l.PushBack(testObject, &TestClass::PrintList, &TestString);

  l.InvokeOperations();

  print("Time has come the walrus said: ", 12.33, "\n\r");

  return 0;
}
