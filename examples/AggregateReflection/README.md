# C++17/C++20: Reflection of Aggregate-initializable Structures

*May 9^th, 2026 By Zeeshan Qazi*

Reflection of structures is often required for mechanisms such as serialization, remote procedure calls, and other similar paradigms, particularly to reduce the boiler plate code. This was typically achieved in C++ before introduction of reflection as a core language feature either by using some kind of a preprocessing tool for generating code or by using a using a library such as [`boost::pfr`](https://www.boost.org/doc/libs/1_84_0/doc/html/boost_pfr.html).

Boost PFR achieves this feat by making use of aggregate initializers to figure out the number of fields, then it uses structured bindings to extract references to the fields in the structure, and figures out the names of the fields by using the external linkage to fields via the same structured bindings in a `consteval` context through a `template &lt;auto&gt;` argument passed to a function template. Aggregate initializer usage to figure out number of fields is possible since C++11, structured bindings are available as a language feature since C++17, and the ability to use `consteval` contexts to get names of individual fields in the structures via external linkage since C++20. 

To help us understand how Boost PFR achieves these feats, I had asked Google Gemini to describe the implementation to understand how it is being done. The explanation offered was:

 

- Aggregate Initializers are used with a dummy type that is convertible to anything in a `decltype` context to make use of SFNAE to figure out how many individual components are allowed in aggregate initialization of the type being reflected.
- This is paired with a binary search mechanism to essentially figure out the total number of elements that are allowed as part of the aggregate initialization of type.
- Once the total count of fields has been discovered some macro magic can be used to form the correct structured bindings initialization to extract the references to the fields in the structure.
- This same mechanism can be used in C++20 with `template` variables at static storage scope with the `template &lt;auto&gt;` method to exctact the field names.

We can basically build this functionality as described by Google Gemini by following these steps:

- **Count the Total Number of Fields**   Search exponentially by passing individual initializers to the aggregate constructor doubling them each time until initialization fails.
- Once the initialization fails search using binary search for number of fields, where the initialization would last suceed.

 
- **Compose a Structured Binding to Extract References to the Fields**   Using macros that are designed to repeat expressions construct structured bindings, which can be used to bind all the fields in the structure.
- The same macros can then be used to create a `std::tuple` via `std::tie` function.
- A function can then be used to extract tuple with references to all the fields in the structure.

- **C++20 and beyond: Get the Names of the Fields by using `template &lt;typename&gt;` Variables**   In C++20, it is possible to use template arguments specialized to a pointer in this case referring to external linkage on fields of structures that are part of external linkage themselves.
- Doing so in a `consteval` context allows you to achieve this without actually generating an aggregate object, or running into linkage errors.
- This allows us to call template functions with pointers to these variables, and then use of `__PRETTY_FUNCTION__` macro to essentially extract the field name. 

Let us look at our implementation, which requires at least C++17:

- `GetStructFieldCount&lt;T&gt;` template variable – evaluates to total number of fields in an aggregate structure.
- `struct_to_tuple` template function – returns a bound `std::tuple` tied to all the fields in the passed instance of aggregate structure.
- `get_field_names` template function – returns on C++20 the array of all the fields in the aggregate structure, otherwise returns synthethic description of the field.
- `for_each_member` template function – iterates through each field in an aggregate structure and passes on the field name and the value to a functor.
- `for_each_member_value` template function – iterates through each field in an aggregate structure and passes the value to a functor.

## C++17 Execution

```bash
host:~$ ./test_struct_reflector
Fields:
	Field [0] @ 0 : std::__cxx11::basic_string, std::allocator >
	Field [1] @ 32 : int
Fields + Values:
	Field [0] @ 0 : std::__cxx11::basic_string, std::allocator > = Zeeshan
	Field [1] @ 32 : int = 45
Just Values:
	[0] = Zeeshan
	[1] = 45
```

## C++20 Execution

```bash
host:~$ ./test_struct_reflector 
Fields: 
	name
	age
Fields + Values: 
	name = Zeeshan
	age = 45
Just Values: 
	[0] = Zeeshan
	[1] = 45
```

[   repeat_macro.hpp](javascript:DoLink('/download-sourcecode.php?Example=repeat_macro');) 
[   struct_reflector.hpp](javascript:DoLink('/download-sourcecode.php?Example=struct_reflector');) 
[   test_struct_reflector.cpp](javascript:DoLink('/download-sourcecode.php?Example=test_struct_reflector');) 

- **G++ C++20 Compilation Instructions:** `g++ --std=c++23 ./test_struct_reflector.cpp -o test_struct_reflector`
- **G++ C++17 Compilation Instructions:** `g++ --std=c++17 ./test_struct_reflector.cpp -o test_struct_reflector`

---
