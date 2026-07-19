# Harnessing Execute Around Pointer Idiom

*December 18^ th, 2016 By Zeeshan Qazi*

I stumbled upon a fairly neat feature in C++, which has existed for a while related to overloading of the `**operator -&gt;**`. This operator may be overloaded as a member of any `**class**` or `**struct**`. The overload is allowed to return a pointer-type or another type that has an `**operator -&gt;**`. If this overload returns another type that has an overloaded `**operator -&gt;**`, it gives us access to a C++ coding idiom called Execute Around Pointer.

This idiom apparently works by automatically chaining **`operator -&gt;`** across multiple return types of multiple levels of overloaded **`operator -&gt;`**, thereby allowing creation and destruction of these types to surround access to the *pointer's* member accessed through the operator invocation. This lends well to providing common code that can execute around accesses to data members or methods in the returned object pointers returned by the last overloaded `**operator -&gt;**` in the chain of these operators.

This idiom can be used to create pointer wrappers, that run specialized code before and after member derefrences. This mechanism effectively provides opportunity to track dereferencing operations of the said pointer wrappers across their entire lifetime, giving us the ability to implement features such as:

- Limited ability to implement certain aspects of *Aspect Oriented Programming (AOP)* in C++.
- Fragmented Managed Shared Memory or Managed Memory Mapped File Segments, allowing lightweight tracking operations for pointers internal (or external) to these structures to permit moving or resizing these structures as need be in a fairly seamless manner.
- Analogs for Marshal By Reference based remote object access.
- Atomic tracking of access to pointed objects (as required by user-land *RCUs* etc.) or implicit locking or logging of these accesses.
- Transactional memory scoping or emulation guarded by the said dereferences.
- Testing guards as part of unit testing suites.

I have utilized this pattern before for Managed Memory Mapped File operations by providing a specialized "smart" pointer like constructions that allow seamless tracking of objects in Managed Memory Mapped File Segments (implemented using **`boost::interprocess`**) across resizes / moves of such file segments. I found that it lended well to providing easy to read code that hid synchronization primitives, access counters etc. from the users. This effectively allowed me to implement a recursive Complex Parent-Child Object tree to keep track of such objects across restarts (or crashes) of a process, effectively allowing the process to maintain detailed state without requiring complex recursive queries to a database.

In this article, we will demonstrate a simple example of usage of this pattern, to help us understand how this mechanism actually works. In the code listing below you can see that:

- `**ProtectedPointer&lt;T&gt; template class**` – That provides a wrapper for `**shared_ptr**` that executes a functor passed to it.
- `**ProtectedPointer&lt;T&gt;::PointerWrap class**` – This internal class is constructed before the access to any member to objected pointed to by the shared pointer accesssed via the previous class, and destroyed right after such access. 

[   Download](javascript:DoLink('/download-sourcecode.php?Example=eap_idiom');)
   Compilation Instructions:  g++ eap_idiom.cpp -std=c++11 -o ./eap_idiom

*Refer to the source file `eap_idiom.cpp` in this directory.*

This output of the above example may appear as below:

`
- ```bash
[zeeshan@localhost ~]$ ./eap_idiom
before x = 0 y = 0 z = 0
after x = 13 y = 0 z = 0
before x = 13 y = 0 z = 0
after x = 13 y = 136 z = 0
before x = 13 y = 136 z = 0
after x = 23 y = 24 z = 19
before x = 23 y = 24 z = 19
after x = 23 y = 24 z = 20
before x = 23 y = 24 z = 20
after x = 35 y = 24 z = 20
before x = 35 y = 24 z = 20
after x = 35 y = 24 z = 21
```

`

---
