# C++26: Using Reflection to Generate Proxy Classes

*May 30^th, 2026 By Zeeshan Qazi*

C++26 reflection provides us the ability to enumerate methods inside a class. This functionality can be used for generation of proxy objects at the compile-time from compile-time reflection of a class.

For our demonstration, we have compiled our code using GCC 16.1 compiler on Ubuntu Linux. We had built our own GCC 16.1 and enabled it as a default using the alternatives feature:

```bash
$ g++ --version
g++ (GCC) 16.1.0
Copyright (C) 2026 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

Our demo essentially consists of just two elements that are used to implement this functionality:

- **`ProxyMethod` template struct** – This template struct is used to represent the methods that are being proxied.
- **`generateProxy` template function** – This template function is used to generate a proxy object, which generates a call to two functors, one before the call to the method being proxied, and the other after the call to the method being proxied.

The proxy is essentially implemented using `std::meta::define_aggregate` function to add to an incomplete `struct`, zero-sized `ProxyMethod` template objects designed to call the method being proxied and the pre and post functors. We achieve this feat by using template arguments to pass in offsets to other members in the proxy object itself.

In the interest of simplicity, we are not handling methods with identical names but different signatures, as this would add a lot more complexity to our demonstration. We have tested our code using godbolt on two different compilers just to ensure C++26 implementation compatibility:

  

- Clang C++ [https://godbolt.org/z/q6zrxad3h](https://godbolt.org/z/q6zrxad3h)
- GCC C++ [https://godbolt.org/z/aY6bGcT75](https://godbolt.org/z/aY6bGcT75)

## Sample Execution

```bash
~/proxy_generation$ ./proxy_generation 
starting
Method call start : say args: []
Hello Yoda
Method call end : say
Method call start : get_calc args: [12, 14]
Method call end : get_calc
calc = 26
```

[   proxy_generation.cpp](javascript:DoLink('/download-sourcecode.php?Example=proxy_generation');) 

- **G++ C++26 Compilation Instructions:** `g++ ./proxy_generation.cpp -o proxy_generation --std=c++26 -freflection`
- **Clang++ C++26 Compilation Instructions:** `clang++ ./proxy_generation.cpp -o proxy_generation --std=c++26 -freflection -fexpansion-statements`

---
