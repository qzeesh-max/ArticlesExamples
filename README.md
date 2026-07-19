# Unified C++ Examples Bundle

This workspace contains a collection of 22 buildable C++ examples migrated from Zeeshan's articles. The examples showcase various techniques ranging from C++14 template meta-programming, userland RCU, and custom reflection, up to cutting-edge C++26 compile-time reflection features.

## Folder Structure

- **`examples/`**: Contains the source code of all 22 examples.
- **`build.sh`**: Linux build orchestrator.
- **`build.bat`**: Windows (CMD) build orchestrator.
- **`CMakeLists.txt`**: Unified CMake configuration.

## Prerequisites

### For Linux
- C++ Compiler (GCC 14+ or Clang 18+ for C++26 reflection examples)
- CMake 3.10+
- Libraries:
  - **Boost** (headers only)
  - **Lua** (Lua headers and development library)
  - **SQLite3** (SQLite3 headers and library)
  - **liburing** (Linux io_uring helpers; required for `CoroutinesUring` example)

Install on Ubuntu/Debian:
```bash
sudo apt-get update && sudo apt-get install -y build-essential cmake libboost-all-dev liblua5.4-dev libsqlite3-dev liburing-dev
```

### For Windows
- **MSYS2** is required.
- Launch the MSYS2 MINGW64 or UCRT64 environment and install dependencies:

For **MINGW64**:
```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-make mingw-w64-x86_64-cmake mingw-w64-x86_64-boost mingw-w64-x86_64-lua mingw-w64-x86_64-sqlite3
```

For **UCRT64** (modern/preferred):
```bash
pacman -S mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-lua mingw-w64-ucrt-x86_64-sqlite3
```
> [!NOTE]
> `liburing` is a Linux-native library, so the `CoroutinesUring` project will be automatically excluded when building on Windows.

## Migrated Projects Index

| Project Folder | Article Title | C++ Standard | Prerequisites | Summary |
| --- | --- | --- | --- | --- |
| [`AggregateReflection`](examples/AggregateReflection) | **C++17/C++20: Reflection of Aggregate-initializable Structures** | C++20 | None | Reflection of structures is often required for mechanisms such as serialization, remote procedure calls, and other similar paradigms, particularly to reduce the boiler plate code. This was typicall... |
| [`Augmentedredblacktree`](examples/Augmentedredblacktree) | **Indexing into Associative Containers and Augmented Red-Black Trees** | C++14 | None | STL – Standard Template Library is a well-known library from point of view of an experienced C++ Software Engineer. It is an extremely useful library providing many different types of generic conta... |
| [`Constexpr`](examples/Constexpr) | **C++14 constexpr and Compile-time Computations** | C++14 | None | `constexpr`'s were initially introduced in C++11, albeit with a very limited scope. Though they were useful, they could not be used without having to re-factor one's logic into a single `return` st... |
| [`CoroutinesUring`](examples/CoroutinesUring) | **C++ Coroutines: Use case for Kernel io_uring development** | C++20 | Boost, liburing (Linux only) | Co-routines in C++ are a useful tool for writing easy to read sequential code, without having to write complex state machines, particularly for use cases where readability of the control flow is im... |
| [`Cpp26Annotations`](examples/Cpp26Annotations) | **C++26: Annotations and Reflection: Custom Attributes** | C++26 | None | C++26 reflection introduces the ability to add and query custom annotations (attributes) on types and fields. This allows developers to attach rich metadata to structures at compile-time and effort... |
| [`Cpp26FeatureDemo`](examples/Cpp26FeatureDemo) | **C++26: #embed Directive and Compile-Time Reflection: Implementing Database Serialization** | C++26 | SQLite3 | C++26 introduces two new features of interest, which are the `#embed` directive and compile-time reflection. In this article, we will show you how to put these features together to essentially gene... |
| [`Cpp26ProxyGeneration`](examples/Cpp26ProxyGeneration) | **C++26: Using Reflection to Generate Proxy Classes** | C++26 | None | C++26 reflection provides us the ability to enumerate methods inside a class. This functionality can be used for generation of proxy objects at the compile-time from compile-time reflection of a cl... |
| [`Crtperasure`](examples/Crtperasure) | **CRTP and Homogeneous Storage via Type-Erasure** | C++14 | None | CRTP (Curiously Recurring Template Pattern) is described as a mechanism for avoiding having to have slow virtual functions dispatched from a base class. Only problem one runs into with CRTP is that... |
| [`Eapidiom`](examples/Eapidiom) | **Harnessing Execute Around Pointer Idiom** | C++14 | None | I stumbled upon a fairly neat feature in C++, which has existed for a while related to overloading of the `operator -&gt;`. This operator may be overloaded as a member of any `class` or `struct`. T... |
| [`Implementingplugins`](examples/Implementingplugins) | **Implementing Plugins in C++ Using Exemplar Pattern** | C++14 | None | Plugins are a very useful feature for extending functionality of many applications. There are a variety of ways plugins may be implemented in C++ applications, such as simple ad-hoc pre-defined fun... |
| [`Luabridge`](examples/Luabridge) | **Implementing a User-friendly C++-Lua Bridge using Variadic Templates and Partial Specialization** | C++14 | Boost, Lua | Lua is a lightweight programming language that is designed to be a multi-paradigm scripting language, explicitly intended to support extensible semantics. Lua is intended to be used either as a sta... |
| [`Orderingstaticinitialization`](examples/Orderingstaticinitialization) | **Ordering Static Initialization By Dependencies** | C++14 | None | Static variables in C++ whether they be global or class scoped statics or locally scoped (even magic) statics may suffer from indeterminable order of construction and/or destruction. This is usuall... |
| [`Overridingvtableatruntime`](examples/Overridingvtableatruntime) | **Overriding Virtual Method Table at Runtime** | C++14 | None | VMT–Virtual Method Table is a construct that is used in C++ to support functionality such as inheritance by providing a table of virtual method pointers for each class that has virtual methods, so ... |
| [`Pointertomemberswithvariadic`](examples/Pointertomemberswithvariadic) | **Pointers to Member Fields With Variadic Templates** | C++14 | None | The variadic templates are a very useful mechanism for specializing generic classes and methods while allowing them to accept an arbitrary number of types and constant literal arguments. But a key ... |
| [`Pointertomembertemplatearguments`](examples/Pointertomembertemplatearguments) | **Using Pointers to Members as Template Arguments** | C++14 | None | Templates in C++ provide very valuable functionality by giving the programmer ability to create generic classes and functions that may be specialized to work with specific classes, and even literal... |
| [`Powerofvariadictemplates`](examples/Powerofvariadictemplates) | **Power of Variadic Templates** | C++14 | None | Variadic templates that were introduced officially in [C++0x standard](https://en.wikipedia.org/wiki/C%2B%2B11) are a very powerful tool that has made meta-programming in C++ a lot easier. Variadic... |
| [`Rcuuserland`](examples/Rcuuserland) | **Implementing RCUs in Userland** | C++14 | None | RCU – Read Copy Update is a mechanism for SMP synchronization that eliminates (or reduces) the need for spinning or blocking to gain access to a resource while allowing safe access to resources sha... |
| [`Reflection`](examples/Reflection) | **Implementing Reflection in C++** | C++20 | Boost | In software development, the term reflection is used to describe ability of a program to inspect or use program elements such as types, classes or methods at runtime such that fluid logic can be bu... |
| [`Sortablerandomaccesstree`](examples/Sortablerandomaccesstree) | **Building a Sortable Random Access Tree** | C++14 | Boost | Though `boost` and `STL` both provide a variety of containers, sometimes it becomes necessary to design your own containers, simply because the containers that are provided are simply not meant to ... |
| [`Templateauto`](examples/Templateauto) | **Using template &lt;auto... &gt; to simplify applying patterns to class members** | C++17 | None | Starting with C++17, it is possible to have [`auto` value arguments as template arguments](https://en.cppreference.com/w/cpp/language/template_parameters), which has opened doors to awesome ability... |
| [`Usingshrarablerobustsynch`](examples/Usingshrarablerobustsynch) | **Using Robust Shareable Synchronization Primitives** | C++14 | Boost | Shared memory is a very fast mechanism for inter-process communication, but this speed comes at a price. Applications that share memory and use this memory to queue and dequeue data between each ot... |
| [`VmtPointer`](examples/VmtPointer) | **Virtual Method Table Internals: Detect Overridden Virtual Methods** | C++14 | None | Virtual Method Tables are a useful mechanism for supporting runtime polymorphism in C++. Though since the virtual methods require that the pointer to the virtual method table be stored in the base ... |

## Building the Projects

### Linux
Run the configuration and build script from the root directory:
```bash
./build.sh
```

### Windows
Ensure the MSYS2/MinGW64 toolchain is in your system `PATH` (e.g. `C:\msys64\mingw64\bin`), and run:
```cmd
build.bat
```
