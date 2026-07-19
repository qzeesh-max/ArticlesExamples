# Using template &lt;auto... &gt; to simplify applying patterns to class members

*September 8^th, 2017 By Zeeshan Qazi*

Starting with C++17, it is possible to have [**`auto`** value arguments as template arguments](https://en.cppreference.com/w/cpp/language/template_parameters), which has opened doors to awesome ability to simplify applying templates to pointer to members at compile-time, which was a lot harder without this ability, since describing a pointer to member as a template argument used to require up to 3 template arguments per pointer to member (i.e. class that contains the member, the type of the member and pointer to member), or cheats using macros, as shown in previous articles on this site. However, this has now been trivialized by allowing us to pass pointer to members directly into template `auto` arguments.

Since template `auto` arguments are open to receive any type of value arguments, to restrict these value to just pointer to members, one would need to use `static_assert`s until `concept`s are added to the C++ language officially. The `static_assert` is combined with `std::is_member_pointer` trait to restrict arguments that are passed into our template class.

Now let us look at how one would implement such a class that utilizes this functionality to trivialize access to member fields (or member methods) passed into this class.

[   template_auto.hpp](javascript:DoLink('/download-sourcecode.php?Example=template_auto');) [   template_auto.cpp](javascript:DoLink('/download-sourcecode.php?Example=template_auto.cpp');)

- **G++ Compilation Instructions:** `g++ template_auto.cpp  -std=c++17  -g3 -O3 -otemplate_auto`
- **Clang++ Compilation Instructions:** `clang++ -std=c++1z template_auto.cpp -g3 -O3 -otemplate_auto`

---
