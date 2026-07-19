# Implementing Plugins in C++ Using Exemplar Pattern

*April 22^ nd, 2015 By Zeeshan Qazi*

Plugins are a very useful feature for extending functionality of many applications. There are a variety of ways plugins may be implemented in C++ applications, such as simple ad-hoc pre-defined functional interfaces, implementation of abstract classes, by using a COM-like discovery-dispatch mechanism or by using a design pattern called exemplar.

Today we will look at one of the simplest way of implementing plugins, which is by means of using the exemplar pattern. The idea behind exemplar pattern is that plugin classes self-register themselves from dynamic libraries such as DLLs or shared objects via the means of global static initializers for an object explicitly dedicated to doing exactly this function. The dynamic libraries are loaded at runtime for this purpose usually due to a reference in an application configuration or presence in a certain folder.

A thing to keep in mind though while writing any distributed library in C++ is the general absence of ABI stability between compilers and sometimes even different versions of the same compiler mainly due to structural differences in classes found in the standard library. This problem severely limits any ability to have presence of STL objects directly in any of the exposed class interfaces from a dynamic library,  even as parameters for any methods. Otherwise, we would require rebuild of the library to work with each compiler version on the same platform. It is however perfectly safe to use STL classes in unexposed implementation of the classes in the library. This is generally achieved with the help of impl pattern, whereby the implementation of the actual class is not exposed but only an opaque pointer to the implementation is present in the exposed interface.

The benefit of using exemplar pattern is that it makes having to discover the methods to call on classes a breeze as the classes themselves self-register and are derived from known interfaces. If more complex or varied functionality is needed then COM style discovery-dispatch interface can easily be implemented.  For what's it worth, you may even combine exemplar pattern implementation with some sort of reflection or runtime retrospection mechanism like the one demonstrated in an article on this site to implement fairly complex and extensible libraries.

Our implementation of the plugin using the exemplar pattern involves a base class for plugin classes, which allows the host for the plugin to be able to invoke methods therein. In our implementation, the ultimate base class for the plugin classes is **`IPluginBase`**. The plugin itself is derived from a template class **`PluginBase`**, which is derived from **`IPluginBase`**. The purpose of the **`PluginBase`** template is to provide implementation of certain virtual methods based on known facts about the plugin.

There are two macros provided to help in declaration of plugin class in the plugin library, namely **`DEFINE_PLUGIN`** to define a class statement before the class body for the plugin along with its requisites, and **`IMPLEMENT_PLUGIN`** effectively meant to register the plugin. **`IMPLEMENT_PLUGIN`** uses the **`PluginRegistration`** class to register itself with the singleton **`PluginManager`**. We also allow querying of the state of the known plugins in our implementation without having to instantiate any of them, and to facilitate this we provide a separate **`PluginInfo`** class, whose instances are available via **`PluginManager`** class.

Our demonstration consists of two dynamic libraries, namely **PluginManager** representing the library providing the ability to implement and consume plugins, and **TestPluginOne** as an example plugin. The main application only sets up dynamic linking with **PluginManager** at compile-time and loads **TestPluginOne** dynamically at runtime. We have implemented this example to be C++11 compliant on Linux but on Windows we have chosen to use **`boost`** for some of the features. We have worked around absent magic statics on some compilers on Windows by using **`std::atomic_flag`**.

**DynamicLibrary.hpp** header file provides ability to load dynamic libraries for both Linux and Windows.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=DynamicLibrary');)

*Refer to the source file `DynamicLibrary.hpp` in this directory.*

**IPluginBase.hpp** header file provides the ultimate base class for all the plugins.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=IPluginBase');)

*Refer to the source file `IPluginBase.hpp` in this directory.*

**PluginBase.hpp** provides the template class which serves as a base class for the plugin classes and implements methods for returning some of the known aspects of the plugin.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=PluginBase');)

*Refer to the source file `PluginBase.hpp` in this directory.*

**PluginInfo.hpp** provides class for keeping track of meta-information for our plugins.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=PluginInfo');)

*Refer to the source file `PluginInfo.hpp` in this directory.*

**PluginManager.hpp** provides the interface for the `**PluginManager**` class that provides ability to plugin host to inquire about the loaded plugins and/or construct them.

 

[   Download](javascript:DoLink('/download-sourcecode.php?Example=PluginManager');)

*Refer to the source file `PluginManager.hpp` in this directory.*

**PluginMethods.hpp** defines the typedefs for function pointers that are used to construct or destruct the plugins.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=PluginMethods');)

*Refer to the source file `PluginMethods.hpp` in this directory.*

**PluginRegistration.hpp** provides the `**PluginRegistration**` template class that registers the plugin class in the plugin dynamic library with the **`PluginManager`** singleton.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=PluginRegistration');)

*Refer to the source file `PluginRegistration.hpp` in this directory.*

**PluginManager.cpp** contains the actual implementation for the **`PluginManager**` class which is kept separate to keep STL library from being exposed on public interfaces of the library to keep the ABI clean.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=PluginManager.cpp');)

*Refer to the source file `PluginManager.cpp` in this directory.*

**plugin.cpp** is the main module for the host application that loads the test plugin.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=plugin');)

*Refer to the source file `plugin.hpp` in this directory.*

**PluginInfo.cpp** contains the implementation for the **`PluginInfo`** class.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=PluginInfo.cpp');)

*Refer to the source file `PluginInfo.cpp` in this directory.*

**TestPluginOne.cpp** contains the main module for the test plugin.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=TestPluginOne');)

*Refer to the source file `TestPluginOne.hpp` in this directory.*

The compilation instructions for the example in this project will generate three binaries:

- PluginManager Dynamic Library
- TestPluginOne Dynamic Library
- PluginTester Application

The compilation instructions for Linux are as follows:

`
```bash
g++ -shared --std=c++11 ./PluginInfo.cpp  PluginManager.cpp -g3 -O3 -fPIC -olibPluginManager.so
g++ -shared --std=c++11  TestPluginOne.cpp  -g3 -O3 -fPIC -olibTestPluginOne.so
g++ --std=c++11  plugin.cpp -L./  -lPluginManager -g3 -O3 -ldl -oPluginTester
```
`

The compilation instructions for Windows are as follows:

`
```bash
g++ -shared --std=c++11 -DUSE_BOOST -DPLUGINDLL_EXPORT PluginInfo.cpp  PluginManager.cpp -g3 -O0  -olibPluginManager.dll -lboost_system
g++ -shared --std=c++11 -DUSE_BOOST -DPLUGINDLL_EXPORT TestPluginOne.cpp -g3 -O0  -olibTestPluginOne.dll -L./ -llibPluginManager -lboost_system
g++ --std=c++11  plugin.cpp -L./  -llibPluginManager -g3 -O0 -oPluginTester.exe
```
`

Sample output from the example:

`
```text
name = TestPluginOne
 address = 0x1b7b850
[0]: PluginClassName= TestPluginOne
Hello from InitializePlugin
Bye from InitializePlugin
```
`

---
