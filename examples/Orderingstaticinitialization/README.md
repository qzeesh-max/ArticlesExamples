# Ordering Static Initialization By Dependencies

*April 17^ th, 2015 By Zeeshan Qazi*

Static variables in C++ whether they be global or class scoped statics or locally scoped (even magic) statics may suffer from indeterminable order of construction and/or destruction. This is usually not a problem, if there is no dependencies between statics, but becomes a problem as soon as such dependencies occur, especially when the application is intended to behave correctly on termination and in case of global or class scoped statics even at the time of initialization. Magic statics or a mutex wrapped local static is often used to solve the problem of indeterminate order of initialization of statics, but still leaves you with indeterminate order of destruction of statics. Magic statics are still not available on Microsoft's Production compilers, eventhough they have been present on GC++ for years.

Typically, Singletons in C++ are implemented either using global or class scoped statics or locally scoped (even magic) statics. These singletons can function properly as long as there is no dependency on another singleton. But as soon as dependencies exist between them, there is no guarantee that global or class scoped statics will be initialized in the correct order, nor is there any guarantee that local scoped statics will be destroyed in the correct order with regard to their dependencies.

Within the same translation unit, the order of the static variables as declared in global scope (including position of the actual class scoped static definition) guarantees that they would be created in that order and destroyed in the reverse order, but all bets are off as soon as there are multiple translation units involved, as the order of statics between them is generally dependent on linkage and often beyond the control of the programmer. These dependencies between translation units can effectively exist just as a matter of need for moduler programming.

In this article we will not try to solve the problem of ordering of both the global or classed scoped statics as well as the local scoped statics. But we will try to demonstrate a solution for the global or class scoped statics only. The solution for the local scoped statics involves fair bit of thread synchronization and various other considerations, including but not limited to availability or non-availability of magic statics, therefore is beyond the scope of this article. Our solution would be implemented using C++ 2011 standard, so to make use of features such as lambda expressions, mainly to improve the readability of the macros provided in this code for this purpose, and to minimize the effort especially as it relates to using STL collections.

This solution is designed to detect non-initialization of the **`OrderedStaticInitializer`** class dedicated for enforcing the order of initialization of instrumented statics. The idea is that a global variable called **`StaticInitializer`** is an instance of this class, which may initialize before or after other instrumented statics, and therefore the other instrumented statics will trigger this variable to get initialized before they initialize themselves. 

Instrumented statics are declared in their appropriate context using either **`ORDERED_NONMEM_STATIC_DECL`** or **`ORDERED_MEM_STATIC_DECL`** depending on whether they are a global scoped or a class scoped static respectively. Instrumented statics are defined in a CPP file using either **`ORDERED_NONMEM_STATIC`** or **`ORDERED_MEM_STATIC`** depending on whether they are a global scoped or a class scoped static respectively. Dependency on a different static is declared using either **`STATIC_REQUIRES_MEM`** or **`STATIC_REQUIRES_NONMEM`** depending on whether another class or global scoped static is a dependency of the defined static. The latter two macros are only used in conjunction with macros for definition of the instrumented static, and not with the declaration of scoped statics.

The implementation makes use of the references in C++ to essentially replace the type of a static with a reference to that type pointing to a controlled region in the data section of the program whose initialization is ordered via **`OrderedStaticInitializer`**. The fact that a method in C++ can return a type that can have implicit conversions defined on it is also used to execute the constructor for instrumented statics and all its dependents only after the underlying reference for the instrumented static has been assigned. This technique is utilized using the **`deferred_init_return`** sub-class whose destructor actually creates the instrumented static or registers the fact that it cannot be created because it has dependencies.

 

Now let us look at the source code that has been commented to aid in understanding of the implementation. **`ordered_static_initializer.hpp`** contains most of the implementation of the instrumentation mechanism.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=ordered_static_initializer');)

*Refer to the source file `ordered_static_initializer.hpp` in this directory.*

The **`ordered_static_initializer.cpp`** only contains one global variable that represents an instance of the **`OrderedStaticInitializer`** class that manages all the instrumented statics.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=ordered_static_initializer.cpp');)

*Refer to the source file `ordered_static_initializer.cpp` in this directory.*

The sample program is contained in **`static_order.cpp`**. It basically demonstrates criss-cross order of static dependencies. Notice we do not use **`std::cout`** as effectively we cannot safely assume that it would still be available to us during the call to the destructors. In fact, in our tests on Android GNU C++, it was not available but on plain old Linux it was still available.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=static_order');)   Compilation Instructions:  g++ --std=c++11 ordered_static_initializer.cpp static_order.cpp -o./static_order_test

*Refer to the source file `static_order.hpp` in this directory.*

The sample output for the program would appear to be like the following:

`
```cpp
construct: printer2::Static two
construct: This should initialize first, because second depends on it
construct: This should initialize second, because it depends on first
construct: printer2::Static one
myInt = 12 non_member2 = This should initialize second, because it depends on first	non_member = This should initialize first, because second depends on it
destroy: printer2::Static two
destroy: This should initialize first, because second depends on it
destroy: This should initialize second, because it depends on first
destroy: printer2::Static one
```
`
