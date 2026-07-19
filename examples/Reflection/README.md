# Implementing Reflection in C++

*March 9^ th, 2013 By Zeeshan Qazi*

In software development, the term reflection is used to describe ability of a program to inspect or use program elements such as types, classes or methods at runtime such that fluid logic can be built at runtime around these types, classes or methods without having to write copious amounts of dispatch logic to dispatch calls to arbitrary functions at runtime. Many languages provide reflection as a builtin feature, but many languages simply do not provide this feature. Some languages utilize this feature silently by using concepts such as late-binding of method invocations or field lookups at runtime.

This feature is not found in majority of native C++ compilers. However, some compilers and frameworks provide this feature with aid of a pre-processor or as a builtin feature in the compiler itself. The **QT framework** provides this feature through a pre-processor, which it calls Meta Object Compiler (MOC). The MOC generates meta objects at compile-time for all classes that are derived from `QObject` class and have macro `Q_OBJECT` mentioned in the body of the class. There are other reflection mechanisms that rely on compiler generated debug information to provide reflection, but these mechanisms may expose innards of your programs to unintended audience. 

Though it is nice to use these features right out of the box, but sometimes it becomes necessary to implement them yourself just because one prefers a different interface, wishes to provide additional features or simply for sake of learning the mechanisms behind implementation of such a concept. We will be implementing a simple and easy to use reflection mechanism using features available in C++ 2011 and the boost framework. Since we do not have a parser built to generate appropriate meta objects to represent our classes at runtime, we would also be implementing some syntactical sugar to ease generation of these meta objects. Our implementation would be slightly constrained as we would not be supporting methods with a variety of signatures, but we would be supporting constructors with a variety of signatures with the caveat that this variety only involves variation in number of arguments. To implement these two features though possible using the mechanisms utilized in this article is beyond the scope of this article and requires a bit more code.

Our implementation would be highly reliant on concepts, we have learnt earlier such as variadic templates, type traits and pointer to member functions. These concepts will be used to decompose method and constructor signatures, bind arbitrarily typed parameters to method invocations, and also to do basic type, `const` and `reference` correctness checks at runtime. These checks would obviously entail the use of Runtime Type Information that C++ runtimes provide as a standard feature. One thing to note however is that this Runtime Type Information lacks information about references built on the type or a `const` modifier applied to the type.

We would first write a few helper classes including `**runtime_type_details_t**` class, which would augment the missing information from the builtin `**type_info**` object. This class will allow us to add reference and const attributes to the type information to provide proper checks on calls to methods that are invoked through reflection. We would also be implementing `**typed_reference_t**` class similar to the concept of **`TypedReference`** that exists in C#. The idea is that we wish to bind references to reference arguments or copies to value arguments with their type information so that the argument may be provided to a function call. Aside from these helper classes we would define a bunch of **`typedefs` ** for collections based on these classes.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=reflection_helpertypes');)

*Refer to the source file `reflection_helper_types.hpp` in this directory.*

Now that we have support classes required for passing arguments with type-safety to the methods that we may invoke through reflection, let us move on to building mechanisms for iteration through elements of the list **`Parameters_t`**, while verifying those against the types expected by the method for compatibility and stacking them for passing them to the method being invoked. This class would also be responsible for calling the member to function and as such will have four different interfaces for four different variants of methods we are supporting, ie. void returning non-const methods, void returning const methods, non-void returning non-const methods, and non-void returning const methods.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=reflection_call');)

*Refer to the source file `reflection_call.hpp` in this directory.*

Now that we have everything to invoke methods while loading arguments from a list, we will now move on to creating classes that would represent methods at runtime. These classes will store meta data about methods so that they may be invocable at runtime with full type-safety and relevant error information on failures. We will be implementing **`IMethod`** interface, which shall be implemented by our template classes **`Method`** and **`ConstMethod`**, in this interface you can see two variants of the **`call`** method, one for invoking methods that return a value, and the other for invoking methods that do not. Aside from these methods, there are methods for returning information about the parameters and return type of the method represented by the interface.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=reflection_method');)

*Refer to the source file `reflection_method.hpp` in this directory.*

Now let us move on to the implementation of the class that will store meta data about the constructors of classes being reflected. Like the **`call`** class that we implemented before for passing parameters from a list to a method at runtime, we would now implement a **`construct`** class which would do the same for calls to the constructor for creation of objects. This class will be utilized by our **`Constructor`** class, which is a simple class that just implements a simple **`IConstructor`** interface responsible for providing a **`construct`** method.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=reflection_constructor');)

*Refer to the source file `reflection_constructor.hpp` in this directory.*

We would now move on to writing the class that would represent meta data about classes at runtime. As usual this class will also be a template class implementing an interface for getting information about the class. This interface would allow the caller to construct the class, find its methods, its class name and other such trivial reflection functions. This class is a lot simpler than everything we have encountered so far in this implementation.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=reflection_class');)

*Refer to the source file `reflection_class.hpp` in this directory.*

Now that we have classes that represent meta data for methods, constructors and classes, we need to have a collection where this meta data may be stored and referenced from. We would be calling this new class **`Module`** and it will basically be a collection of objects of **`Class`** type that was implemented previously.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=reflection_module');)

*Refer to the source file `reflection_module.hpp` in this directory.*

We shall write the final header file of our implementation. This header file is the glue that will bring together all the classes we have implemented with the syntactical sugar that we promised would ease the use of these classes. In this header file you will find macros designed to act as blocks so that they are scoped. You will see how to use these macros in our example below. 

[   Download](javascript:DoLink('/download-sourcecode.php?Example=reflection');)

*Refer to the source file `reflection.hpp` in this directory.*

Now let us move on to our example that uses the classes we defined so far, you would notice in this example that we use **`export_module`**, **`insert_class`**, and other macros defined in the reflection header. You will notice in the example that we use these macros with minimal effort whereas the calls to the underlying functions are a bit more complicated.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=reflection_example');)
   Compilation Instructions:  g++ -std=c++11 reflection_example.cpp -o reflection_example

*Refer to the source file `reflection_example.hpp` in this directory.*

So this was a relatively simple example of an implementation of reflection. You would notice that while passing the reference arguments through the `**Parameters_t**` collection we call `**boost::ref**` function to get a `**reference_wrapper**` object that allows us to by pass ambiguities in constructor signature of the `**typed_reference_t**`.
