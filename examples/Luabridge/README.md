# Implementing a User-friendly C++-Lua Bridge using Variadic Templates and Partial Specialization

*August 23^ rd, 2015 By Zeeshan Qazi*

Lua is a lightweight programming language that is designed to be a multi-paradigm scripting language, explicitly intended to support extensible semantics. Lua is intended to be used either as a standalone scripting language or as a scripting engine in another program. Lua provides a rich C interface for exposing C functions to Lua and gaining access to the Lua objects from C. However, this interaction with Lua is massively dependent on the ability of a person to understand low-level stack-based programming model, where the arguments, results and even target sites of calls are pushed onto a stack.

Here are the key differences between Lua and C++:

- Lua manages memory of all its objects using garbage collection, where as dynamic objects allocated on heap in C++ are required to be managed by the programmer by default unless a third-party library is used for memory management that supports this handling.
- Lua is not a strongly typed language and uses dynamic typing, where as C++ is a strongly typed language, with a concept of type polymorphism.
- Lua does not support allocating complex objects in a scoped region such that they would be destroyed on exit from the region, where as C++ does not have such a constraint.
- Variables do not need to be declared in Lua before their first use, and variables are always global unless declared explicitly as local, where as C++ has proper scoping for variables and requires them to be declared.
- Using meta-tables in Lua, it is actually possible to implement some semblance of classes and class hierarchies, where as in C++ these things are quite readily available.
- Lua error handling is based on setjmp/longjmp, and is painful to use with C++ objects, as it interferes with C++ object destruction. C++ on the other hand does proper destruction of its objects when an exception is thrown. 

Today, we are going to implement a fully functional bridge between Lua and C++ that allows a trivial interface to be used to expose C++ classes and methods to Lua. For our implementation today, we will support the following functionality:

- Ability to export a C++ class with its single constructor to Lua with any of the desired methods each with single signature (unless two different signatures are named differently as visible from Lua).
- Ability to export a C++ function that is not a member of a class.
- Ability to receive as argument to C++ function or method parameters of primitive types, strings, Lua functions, pointers to C++ classes or structures, or boost::shared_ptr's to C++ classes or structures.
- Wherever boost::shared_ptr is used, automatic handling of lifecycle of the object pointed to by that boost::shared_ptr in Lua state concurrently with the C++ program, such that collection of Lua object wrappers for boost::shared_ptr-ed object causes destruction of the object in C++, if there are no references left in C++.
- Ability to store and call Lua functions passed to C++ programs from within Lua programs.
- Ability to catch Lua errors thrown by Lua functions called from C++ program like regular exceptions, and to convert C++ exceptions to Lua errors to be caught by the Lua program.
- Type checking of arguments passed to C++ functions from Lua, and throwing appropriate errors catchable in Lua to indicate a problem.
- Automatic conversion of reference arguments in C++ methods and functions to additional return values retrievable from Lua.
- Access to not just the first but all the return values of a Lua function call from within C++.

We will not be supporting handling of arbitrary tables passed in from Lua as part of this tutorial due to complexity of the interface required to interact with arbitrary tables. However, since we are representing C++ classes as simple tables in Lua that contain information about C++ objects, we would be handling those tables.

Our implementation will expose pointers to `struct` and `class` types as tables with the following fields:

- `__cpp_data` – pointer to the C++ struct or class.
- `__cpp_type` – pointer to the type_info for struct or class.

Furthermore, our implementation will handle classes constructed via the exposed interface by providing `boost::shared_ptr` to the instance of those classes.  These `boost::shared_ptr` will be returned wrapped in tables with the following fields:

- `__cpp_data` – pointer to the boost::shared_ptr.
- `__cpp_type` – pointer to the type_info for boost::shared_ptr.
- metatable field __gc – pointer to the destruction function.
- other fields – Pointers to closures wrapping methods for the classes, callable from Lua.

Weak references to `boost::shared_ptr` to the objects returned via C++ are kept in a per class table in the Lua state. These weak references are removed when the object no longer exists in Lua state, in which case a new `boost::shared_ptr` copy for Lua state would be created if the object pointed to by the deleted `shared_ptr` were to be returned back to Lua. If at the time of the removal of weak reference, there are no references to the object in C++ instance, then the object is destroyed.

Our implementation contains a template class `lua_choose_type` template class that is used to select appropriate behavior for a method argument or return value while interacting with Lua. The various partial specializations of this class are used to select appropriate behavior with regard to C++ argument types. Our specializations also contain a mechanism for detection of reference arguments in C++ methods and funtions so those arguments may be converted into additional return values from the point of view of Lua caller. Template class `reference_detector` aids in this reference argument detection, and handles pushing of the values of the reference arguments after return from the C++ function.

Class `LuaFunctionWrapper` is used to store a reference to the Lua function in a C++ object. This class also facilitates calling of the Lua function from C++ with appropriate translation of arguments and retrieval of all the returned values. The mediation between the Lua state and the C++ caller for retrieval of return values is done via a class called `lua_returned_values`.

Template classes `WrapFunctionForLua` and `WrapMethodForLua` are used for building appropriate Lua C closures for calling functions or methods. These template classes are available to the users of this library via interface methods `RegisterLuaFunction` and `RegisterLuaMethod`. The C++ classes may be exposed to Lua using the method `RegisterLuaClass` provided for this purpose.

`LUA_CLASS_METHODS`, `LUA_REGISTER_METHOD`, and `LUA_END_CLASS_METHODS` macros may be used with `RegisterLuaClass` to register methods to a class while registering a C++ class with Lua.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=lua_cpp_bridge');)

*Refer to the source file `lua_cpp_bridge.hpp` in this directory.*

We will now demonstrate how to use our library to expose C++ classes and methods. Our example program is as below:

[   Download](javascript:DoLink('/download-sourcecode.php?Example=test_lua');)
   Compilation Instructions:  g++ -O2 --std=c++11 test_lua.cpp -llua -o./test_lua

*Refer to the source file `test_lua.cpp` in this directory.*

Here is a Lua program to interact with the interfaces we have exposed via our C++ example program above:

[   Download](javascript:DoLink('/download-sourcecode.php?Example=test.lua');)

*Refer to the source file `test_lua.hpp` in this directory.*

To execute our example we would just invoke it as below:

`
- ```bash
[zeeshan@localhost lua_overview]$ ./lua_test  

---
