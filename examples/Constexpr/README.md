# C++14 constexpr and Compile-time Computations

*May 27^th, 2017 By Zeeshan Qazi*

`constexpr`'s were initially introduced in C++11, albeit with a very limited scope. Though they were useful, they could not be used without having to re-factor one's logic into a single `return` statement per `constexpr` function, often requiring recursion to be used. Common compilers lacking tail call optimization while evaluation those expressions also resulted in substantial constraints on their usability, without using some form of exponential recursion with continuation passing (see [CPS](https://en.wikipedia.org/wiki/Continuation-passing_style)) to step through the algorithm.

But since then things have substantially changed, and C++ has much more capable `constexpr` methods possible since C++14, and ability to even have `constexpr` lambda's since C++17. Though the compiler support is still fraught with performance constraints, and bugs, these statements can still be used carefully to implement fairly complex computations at compile-time, such as computation of complex algebraic expressions for Matrix calculations, variant encodings for strings, especially, using new `constexpr` based string literal operators, and many other similar useful functions.

There are quite a few documented bugs that exist in GNU C++ Compiler when it comes to evaluation of `constexpr` functions. So one has to tread carefully when writing these, often to work around performance problems with the compiler. Here are a few bugs that one may come across:

- [Wrong Code with Post-Increment Operator in ConstExpression](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=77553)
- [Regression in C++ parsing performance between 4.9.3 and 5.3.1](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70452)
- [Gcc uses large amounts of memory and processor power with large C++11 bitsets](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56671)
- [Spurious caching for constexpr arguments](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79520)
- [G++ uses up all my RAM when compiling a constexpr with exponential call graph](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55442)

Clang++'s behavior is a lot better in this regard, at least in that which I have observed in my testing. However, people have reported compiler crashes with certain `constexpr` constructions even with Clang++ particularly those involving exponential recursion. The chief complaint though remains with G++ where it seems to be treating even the for-loops as recursive calls, and having exponential growth of memory usage with passage of time. Exponential Recursion with Continuation Passing Style (CPS) barely helped with the memory growth issues.

I have used `constexpr`'s in the demonstration program provided in this example to show their usefulness for a "near" real-world task. String processing is one of those tasks that often takes a toll on performance of one's applications. Strings in C/C++ are not usable in switch/case statements right off the bat, and dispatching on them in O(1) or nearly O(1) complexity, is often a problem. 

I usually resolve to use [gperf](https://www.gnu.org/software/gperf/) (Perfect Hash) utility to generate perfect hashes whenever I already know the set of strings that I will encounter in processing of a certain text-based protocol on wire, or any protocol that uses text-based field names, or for that matter anything involving large number of previously known keywords requiring performance.

Using gperf does give one major performance improvements, but at a cost of having to maintain separate gperf input files, either via some scripts or manually.

*Would it not be nice if the logic similar to the gperf library could be implemented inside the program itself so these hashes could be computed at compile time?*

Yes, constexpr do provide us some limited ability to do the same, to an extent, but compiler implementation imposed constraints to throw a wrench in the works, although at perhaps a minimal start-up cost the same expressions could be used inside non-constexpr static const global members to carry the same hash algorithm.

For our example we will be implementing a near-perfect hash using logic somewhat similar to gperf, with a few simplifications keeping the performance constraints of the compilers in mind. Our hash would basically be computed in the following manner:

- We will walkthrough all the strings in the set counting each unique character at all the positions in the string, and we will compute the minimum string length in the set, and the average length for all the strings in the set.
- We will then select the most frequent unique characters based on the descending sort, such that, this relationship exists between the two ratios:

- The ratio of characters chosen at positions at or below the minimum length to the characters chosen at or below average length.
- **is greater than or equal to** the ratio of minimum length to average length of strings in the set.

- We will hash all the strings using the characters we chose, the last character in each string and the length of the string using the [Fowler-Noll-Vo Hash Function](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function), specifically **FNV-1 hash**.
- We will make an associative array whose index would represent the hash, giving us O(1) access [for non-collision cases] and no more than O(n) [where n is substantially smaller than total count of strings] for collision cases in order to map a string to an enumeration.

So before we get coding let us understand what we would need for this to work:

- All compiler versions do not yet implement initialization_list with `constexpr`. We have worked around this to support older versions by taking variadic template arguments instead.
- We had also noticed some issues with what sized arrays did or did not support between various versions of compilers, and implementations of STL algorithms header. So we had come up with a thin wrapper around the array to provide this functionality for `constexpr`. We also ended up implementing some basic algorithms for `constexpr`'s while doing so.

Our demonstration today is broken into four files:

- `**cx_helpers.hpp**` – The helper methods that provide `constexpr` implementations for basic algorithms, arrays, pairs, and apparatus to support exponential recursion with CPS.
- `**near_perfect_hash.hpp**` – Implementation of Near Perfect Hash, a "simplified" version of the gperf hashing algorithm. This algorithm may be used as constant expressions to compute hashes at compile-time, or alternatively at static initialization time. Of course compile-time usage would eliminate constant expression calls to just the results.
- `**NearPerfectHash.cpp**` – An example of the usage of the header files.
- `**fix_enums.h**` – An example of all the FIX field names hashed using the `constexpr`-based Hashing algorithm. An example of its usage is shown in `NearPerfectHash.cpp`.

Here are the files to get us started along with compilation instructions:

[   near_perfect_hash.hpp](javascript:DoLink('/download-sourcecode.php?Example=near_perfect_hash');) [   cx_helpers.hpp](javascript:DoLink('/download-sourcecode.php?Example=cx_helpers');) [   fix_enums.h](javascript:DoLink('/download-sourcecode.php?Example=fix_enums');) [   NearPerfectHash.cpp](javascript:DoLink('/download-sourcecode.php?Example=NearPerfectHash');)

- **G++ Compilation Instructions:** `g++ -DUSE_CONSTEXPR_FOR_HASH  NearPerfectHash.cpp  -std=c++14  -g3 -O3 -oNearPerfectHash`
- **Clang++ Compilation Instructions:** `clang++ -ftemplate-depth=1024 -DUSE_CONSTEXPR_FOR_HASH  -std=c++14 NearPerfectHash.cpp -g3 -O3 -oNearPerfectHash`

If you notice the compilation instructions that there is a `-ftemplate-depth` argument passed to `clang++`, this is because we need recursion to extract the variadic argument list. Likewise, a compiler may need you to provide additional arguments such as `-fconstexpr-depth` to control the `constexpr` recursion depth or `-fconstexpr-steps` to control the number of steps. Both GCC and Clang++ support identical set of flags, except GCC seems to have no limit on number of steps per say and their template recursion depths is higher by default (at 900) for template compare to Clang's (at 256).

### Compile-time Objects: Hashing Strings

In the `near_perfect_hash.hpp`, you would notice that we have a class called `EnumValueNameConstruct`. An instance of this class is returned by the free function `make_enum_name`. The idea behind this class is to allow perservation of the length of the compile-time string literal, and make it available to be passed to not just `EnumValueName` class constructor, but also with the compile-time string in a variadic pack to the `cx_hash` class constructor.

Upon the construction of `NearPerfectHash` class, `cx_hash` is also constructed as a member inside the class instance, and an array inside the `NearPerfectHash` instance also receives newly constructed `EnumValueName` for each argument received. The `cx_hash` class makes a histogram of count of each character at each position in the string set, and also a histogram of count of unique characters at each position.

The latter histogram that is created is used to select positions in the strings (as described in the above algorithm), and a member `scounts` in `cx_hash` contains sorted string positions by descending unique character count. And this member is used in the `NearPerfectHash` class to compute hashes to populate the hash table and a cheat table to allow O(1) lookups for the first entry with the same hash. The cheat table also contains the count of strings at each hash.

### So what happens when you lookup?

Lookups in the `NearPerfectHash` are by string, and yield the `enum` value that is associated with the string at the time of construction. The string is hashed using our hash algorithm using the positions provided. The key though is the loop in the `Convert` method would normally disappear due to `constexpr` nature of the data involved, replacing it with explicit index lookups into the strings, and the entire algorithm would disappear and get replaced by the results, in the cases where the argument to `Convert` is also a `constexpr`.

So as an example let's look at two `constexpr` strings passed to convert at following lines in `NearPerfectHash.cpp`:

`

- **107** `std::cout (FixTagHash.Convert("Account", static_cast(0))) (FixTagHash.Convert("Text", static_cast(0))) :	mov    $0x1,%esi
   0x40108a :	mov    $0x6030c0,%edi
   0x40108f :	callq  0x400a80 
   0x401094 :	mov    %rax,%rdi
   0x401097 :	callq  0x400b70 
   0x40109c :	mov    $0x13,%edx
   0x4010a1 :	mov    $0x401387,%esi
   0x4010a6 :	mov    $0x6030c0,%edi
   0x4010ab :	callq  0x400b10 
   0x4010b0 :	mov    $0x3a,%esi
   0x4010b5 :	mov    $0x6030c0,%edi
   0x4010ba :	callq  0x400a80 
   0x4010bf :	mov    %rax,%rdi
   0x4010c2 :	callq  0x400b70 
```
`

The FIX tag for Account is 1 (0x01), and for Text is 58 (0x3a). So you can see above that the call to convert has been replaced by the enumeration value.

### Quality of Our Hash

Granted that it is not feasible yet due to compiler bugs and performance constraints to implement the full algorithm from gperf in the `constexpr` objects, we were able to achieve fairly decent results. We hashed a total of 405 FIX Tag names and mapped them using an `enum` to FIX Tag numbers, with only 17 collisions and all of these collisions involved a single additional member in the bucket. Our algorithm utilized 6 most frequently different character positions from all the strings, their last character and the length to hash the FIX tag names.

We gained the ability to use the FIX Tag names in a **`switch-case`** statement by doing this.

### Conclusion

`constexpr`s may be used to pre-compute data prior to runtime of the applcation such that the loop iterations and other code that relies on variable data may be eliminated for data that we know would remain constant. This can give substantial performance boost for certain processing tasks where we know that the data needed to 
perform the task is not going to change with an additional benefit of not having to maintain dependencies external to source code for the data or mappings related to it.

Technically, enumerations could be wrapped in macros with their names mapping back to them via a `constexpr` (near) perfect hash.

---
