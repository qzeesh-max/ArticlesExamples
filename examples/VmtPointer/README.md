# Virtual Method Table Internals: Detect Overridden Virtual Methods

*September 8^th, 2018 By Zeeshan Qazi*

Virtual Method Tables are a useful mechanism for supporting runtime polymorphism in C++. Though since the virtual methods require that the pointer to the virtual method table be stored in the base of each object with any virtual method, and that a lookup be performed to find the virtual method table and the pointer to the method in the virtual method table, there is a performance hit caused by use of virtual methods, which may lead to inviability of use of virtual methods in critical path of low latency applications. That is why many people prefer to leave these methods out of highly performant code.

Sometimes one may not have any choice but to use virtual methods but it may be so that these virtual methods though frequently called in your code, may be seldom overridden. So essentially a faster path may be possible for majority of cases where the dynamic dispatch is unneccessary. Sometimes these dynamic dispatches may be prohibitively expensive simply because you may have complex functional overrides overridding certain functionality in your code such as memory allocation, transports etc. using policy-based design based methodologies whereby virtual method may only be used to dispatch to a specialization of templated method via a proxy class.

So in essence, to keep the option of dynamic runtime polymorphism, easily attainable, while not requiring code duplication or writing prohibitively complex code, one may require understanding of the virtual method table and virtual method dispatch.

Pointer to member functions that are created for virtual methods in many mainstream compilers (**GNU C++** and **Clang C++**, in our example) consist of two machine words with essentially the following structure on Intel Linux Platforms (as documented in [Intel Itanium Linux CXX ABI 1.83, &sect; 2.3](https://refspecs.linuxbase.org/cxxabi-1.83.html#member-pointers)):

`

```cpp
union MemberFunctionPtr
{
    virtual-method-pointer  memberPtr;
    struct
    {
        ptrdiff_t memberFunctionPtrOffset;
        ptrdiff_t vmTableOffset;
    };
};
```

`

In the structure that represents a *pointer to virtual method*, the `vmTableOffset` is basically the offset to the pointer to virtual method table inside the object. And `memberFunctionPtrOffset` is basically the offset in that virtual method table ** plus one**, basically a zero in this member means a null pointer to a virtual method.

Let us look at an example of how one may utilize this mechanism to figure out the underlying method, or the fact whether such a method is overridden or not.

**Note to the readers:** The example provided below is only usable on systems that have a particular ABI. ARM Platforms have a different ABI with slightly different implementation, which means that the code needs to be adjusted (or have conditional compilation used) to run with on systems with a different ABI. The description of ARM ABI's implementation of *pointer to members*  may be found in [C++ ABI for ARM Architecture &sect;3.2.1](https://infocenter.arm.com/help/topic/com.arm.doc.ihi0041e/IHI0041E_cppabi.pdf).

[   virtual_method_helper.hpp](javascript:DoLink('/download-sourcecode.php?Example=virtual_method_helper');) [   test_vmt_helper.cpp](javascript:DoLink('/download-sourcecode.php?Example=test_vmt_helper.cpp');)

- **G++ Compilation Instructions:** `g++ test_vmt_helper.cpp  -std=c++17  -g3 -O3 -otest_vmt_helper`
- **Clang++ Compilation Instructions:** `clang++ -std=c++1z test_vmt_helper.cpp -g3 -O3 -otest_vmt_helper`

---
