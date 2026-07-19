# Power of Variadic Templates

*September 25^th, 2011 By Zeeshan Qazi*

Variadic templates that were introduced officially in [C++0x standard](https://en.wikipedia.org/wiki/C%2B%2B11) are a very powerful tool that has made meta-programming in C++ a lot easier. Variadic templates allows template classes and functions to accept multiple arguments relatively easily with type-safety which was previously much harder to do as the programmers had to write much more complex template classes and methods often with a large number of variants to accept largely variable list of arguments.

Variadic templates changed all of that by introducing the concept of parameter packs, which may be expanded easily. A parameter pack for a class or a method may be declared by just using **...** (elipses) in the a template's parameter list. A parameter pack may be expanded by using the **...** (elipses) in a parameter-list of a function declaration, a template parameter in a field or variable declaration, or a parameter-list of function invocation.

**Example 1:** This demonstrates a simple utilization of variadic templates to implement a print function:

`	**void** print()
	{
	  std::cout.flush();
	}
	
	**template** &lt;**typename** *FirstArgumentType*, **typename** **...***ArgumentType*&gt;
	**void** print(**const** *FirstArgumentType*& firstArgument, **const** *ArgumentType*&**...** arguments)
	{
	  std::cout //This declares the base template with variadic arguments to tell the compiler
//that we will be providing it specializations*
**template**&lt;typename... ArgumentType&gt;
**class** ParameterStoreImpl;

*//This specialization allows invocation of the actual method and invocation of this
//specialization actually trickles down through recursive instantiation of the ParameterStoreImpl
//template.*
**template**&lt;&gt;
**class** ParameterStoreImpl&lt;&gt;
{
**public:**
  **template** &lt;**typename** ClassType, **typename** MethodType, **typename**... MethodArgumentType&gt;
    **void** Execute(ClassType& instance, MethodType method, MethodArgumentType**...** arguments)
    {
      (instance.*method)(arguments...);
    }
};

*//This specialization is designed to actually extract the first template argument
//from the variadic template arguments so that a field may be declared to store its value.
//The **Execute** method in this specialization actually concatenates the argument stored in this
//instance of **ParameterStoreImpl specialization** to the list of arguments passed from previous
//invocation of the Execute function in another specialization.*
**template** &lt;**typename** FirstType, **typename...** ArgumentType&gt;
**class** ParameterStoreImpl&lt;FirstType, ArgumentType...&gt;
{
**public**:
  FirstType FirstValue;
  ParameterStoreImpl&lt;ArgumentType...&gt; MoreValues;

  ParameterStoreImpl(FirstType firstValue, ArgumentType**...** moreValues):
    FirstValue(firstValue), MoreValues(moreValues**...**)
  {
  }

  ~ParameterStoreImpl()
  {
  }

  **template** &lt;**typename** ClassType, **typename** MethodType, **typename**... MethodArgumentType&gt;
  **void** Execute(ClassType& instance, MethodType method, MethodArgumentType**...** arguments)
  {

    MoreValues.Execute(instance, method, arguments..., FirstValue);
  }
};
`

Though we just declared a class for storing the arguments for invocation of instance methods, actually initiating this class is a bit more work without a helper function to do so. Here is how one would instantiate and use this class without a helper function. In this example we are also declaring a `TestClass` class in order to demonstrate the use of our final product of this tutorial.So let us try to use this class without a helper method for easing its generation.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=vartempl-ex3');)
   Compilation Instructions:  g++ -std=c++11 vartempl-ex3.cpp -o vartempl-ex3

**Example 3:** This demonstration shows how to use the `ParameterStoreImpl` class we declared in Example 2.

`**class** TestClass
{
**public**:
  **void** TestMethod1()
  {
    std::cout &lt;&lt; "Sing a song of six pence," &lt;&lt; endl;
    std::cout &lt;&lt;"a pocket full of rye," &lt;&lt; endl;
  }

  **void** TestMethod2(int someNumber)
  {
    std::cout &lt;&lt; "The number is " &lt;&lt; someNumber &lt;&lt; endl;
  }

  **void** TestMethod3(**int** someNumber, **const** string& someText)
  {
    std::cout &lt;&lt; "someText: " &lt;&lt; someText &lt;&lt; endl;
    std::cout &lt;&lt; "someNumber: " &lt;&lt; someNumber &lt;&lt; endl;
  }

  **void** PrintList(**const** std::list&lt;std::string&gt;* k)
  {
    std::cout &lt;&lt; " list size = " &lt;&lt; k-&gt;size() &lt;&lt; std::endl;
  }
};

*//Notice the **main** function and how the **ParameterStoreImpl** is 
//declared and how each parameter type has to be provided when declaring the variable.*
**int** main()
{
  TestClass testObject;

  ParameterStoreImpl&lt;**int**,std::string&gt; MyParameter(666, "This is good!");

  MyParameter.Execute(testObject, &TestClass::TestMethod3);

}

`

This need to specify individual parameters may be resolved by providing a helper function, and utilizing the `**auto**`keyword to declare the variable instead.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=vartempl-ex4');)
   Compilation Instructions:  g++ -std=c++11 vartempl-ex4.cpp -o vartempl-ex4

**Example 4:** Declaring a helper function for creating an instance of  `ParameterStoreImpl` class.

`**template**&lt;**typename...** ArgumentTypes&gt;
ParameterStoreImpl&lt;ArgumentTypes**...**&gt;CreateParameterStore(ArgumentTypes... arguments)
{
  **return** ParameterStoreImpl&lt;ArgumentTypes...&gt;(arguments...);
}

**int** main()
{
  TestClass testObject;

  **auto** MyParameter = CreateParameterStore(666, "This is good!");

  MyParameter.Execute(testObject, &amp;TestClass::TestMethod3);

  **return** 0;
}
`

Now let us expand the utility of these classes by developing a `OperationList`, which will allow us to store method invocations within it to call at a later time or to pass them from one thread to another. This class may also be used for optimizing many other operations or to implement the concept of transactions.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=vartempl-ex5');)
   Compilation Instructions:  g++ -std=c++11 vartempl-ex5.cpp -o vartempl-ex5

**Example 5:** Implementing `OperationList` class.

`*//Abstract class for invoking a bounded method stored by the Operation class.*
**class** IOperation
{
**public**:  **virtual void** Execute() = 0;

  **virtual** ~IOperation() {}
};

*//This class is used to bind parameters for a call to a method, and for invoking the method with those parameters.*
**template** &lt;**typename** ClassType, **typename** MethodType, **typename...** ArgumentType&gt;
**class** Operation : **public** IOperation
{
**protected**:
  ClassType& Instance;
  MethodType Method;
  ParameterStoreImpl&lt;ArgumentType...&gt; Arguments;

**public**:
  Operation(ClassType& instance, 
    MethodType method, 
    ArgumentType&... arguments) :
    Instance(instance),
    Method(method),
    Arguments(arguments...)
  {
  }

  **virtual** ~Operation() {}

  **virtual void** Execute()
  {
    Arguments.Execute(Instance, Method);
  }
};

*//This is the OperationList class to which one may queue up function invocations for later execution.*
**class** OperationList
{
**protected**:
  **typedef** std::list&lt;IOperation*&gt; operations_t;
  operations_t Operations;

**public**:
  OperationList()
  {
  }

  **virtual** ~OperationList()
  {
    **for** (operations_t::iterator it = Operations.begin(),
      lastIt = Operations.end();
      it!=lastIt; it++)
    {
    **delete** *it;
    }
  }

**template** &lt;**typename** ClassType, **typename** MethodType, **typename...** ArgumentType&gt;
  **void** PushBack(ClassType& instance, MethodType method, ArgumentType... arguments)
  {
    Operations.push_back(new Operation&lt;ClassType, MethodType, ArgumentType...&gt;(instance, method, arguments...));
  }

  **template** &lt;**typename** ClassType, **typename** MethodType, **typename...** ArgumentType&gt;
  **void** PushFront(ClassType& instance, MethodType method, ArgumentType... arguments)
  {
    Operations.push_back(new Operation&lt;ClassType, MethodType, ArgumentType...&gt;(instance, method, arguments...));
  }

  **void** InvokeOperations()
  {
  **for** (operations_t::iterator it = Operations.begin(),
      lastIt = Operations.end();
      it!=lastIt;	it++)
    {
      (*it)->Execute();
    }
  }
};

*//Here is an example application to show you how to use the above classes.*
**int** main()
{
  TestClass testObject;

  OperationList l;
  std::list&lt;std::string&gt; TestString;

  l.PushBack(testObject, &TestClass::TestMethod1);
  l.PushBack(testObject, &TestClass::TestMethod2, 12);
  l.PushFront(testObject, &TestClass::TestMethod3, 15, "Hello World");

  **void** (std::list&lt;std::string&gt;::*std_list_string_push_back)(**const** std::string&) = &std::list&lt;std::string&gt;::push_back;

  l.PushBack(TestString, std_list_string_push_back, "Test");
  l.PushBack(testObject, &TestClass::PrintList, &TestString);

  l.InvokeOperations();

  print("Time has come the walrus said: ", 12.33, "\n\r");

  **return** 0;
}
`
