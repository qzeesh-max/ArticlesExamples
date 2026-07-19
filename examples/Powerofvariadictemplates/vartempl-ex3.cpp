#include <iostream>
#include <list>

using namespace std;

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

//Notice the main function and how the ParameterStoreImpl is 
//declared and how each parameter type has to be provided when declaring the variable.
int main()
{
  TestClass testObject;

  ParameterStoreImpl<int,std::string> MyParameter(666, "This is good!");

  MyParameter.Execute(testObject, &TestClass::TestMethod3);

}

