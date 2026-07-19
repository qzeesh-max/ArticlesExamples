# Pointers to Member Fields With Variadic Templates

*February 14^th, 2013 By Zeeshan Qazi*

The variadic templates are a very useful mechanism for specializing generic classes and methods while allowing them to accept an arbitrary number of types and constant literal arguments. But a key problem with template arguments while specializing classes is that there is no direct way of causing variadic template arguments to accept compile time constants (such as pointers to member fields) with arbitrary types for each one of the arguments. As a general rule, a template argument in a template class declaration must either be a `typename`, a `class`, or a literal of a predetermined type, and as such variadic template arguments can only be a set of one of these language elements. On the other hand, neither the `auto` keyword nor any other compile time method can provide us with a direct way of allowing us to specify a mixture of differently typed constant literal arguments to a `class` or `method` template. Only workarounds for this problem essentially involve non-intuitive use of automatic type deduction of template arguments, barring which it is a difficult and syntactically ugly problem to solve.

We would look at a variety of solutions to this problem while trying to apply the use of pointer to member fields to a simple problem of creating a template to generate a simple comparison function that takes as arguments a list of pointers to members of a `class` or `struct`. However, this same mechanism can also be used to create templates for birectional serialization, runtime type-retrospection seeding, and even for templatizing object marshaling mechanisms. We are using this simple problem to show the power of variadic templates when combined with pointer to member fields, but this example can also be seen as a method for creating template based tuples of literals of a variety of types all used at once in a single template specialization.

Our first example will use template function argument type deduction to apply arguments types to a template class. Though our example uses `constexpr` keyword to hint to the compiler that under normal circumstances all the return values and initialized classes will be literals so long as call to `Ascending_Compare` is made with literal arguments only, this keyword is unnecessary for anything other than telling the compiler to execute the function or create the objects at compile-time if possible. The compiler in many cases with optimizations enabled will still do the same, even without the `constexpr` keyword. So let us examine a sample program involving this mechanism, and then we would look at the pros and cons of this implementation.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=variadic-ptm-ex1');)
   Compilation Instructions:  g++ -std=c++11 variadic-ptm-ex1.cpp -o variadic-ptm-ex1

*Refer to the source file `variadic_ex_pm1.hpp` in this directory.*

As you can probably discern from the program above, the workaround used here for the variadic template restriction that prevents use of literal arguments with varying types by using the types of those arguments as template parameters via function argument type deduction, and then storing the values of the literals in `const` members of the class. Though this provides a functor object to be callable from STL algorithms or anything that can invoke such at runtime, the functor provided cannot be passed as a template argument itself. Also, the literals are stored in member variables in this case, and depending on optimizations enabled while compiling this may or may not be very efficient way of generating a functor. 

Now let us look at another implementation of the solution to the same problem, where we would actually store the literals in template arguments, while maintaining the ease of use, there by solving the problem of the functor not being usable as a template argument to other classes. This solution would involve use of one macro to provide a cleaner interface to the user.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=variadic-ptm-ex2');)
   Compilation Instructions:  g++ -std=c++11 variadic-ptm-ex2.cpp -o variadic-ptm-ex2

*Refer to the source file `variadic_ex_pm2.hpp` in this directory.*

As you can see from the two examples above that use of pointer to member with variadic arguments is not straight forward, but it is bit of an undertaking. The two implementations above are designed for ease of use, but an easier implementation would not be as seemless to use as above. Technically, it is possible to implement the above by using a simple wrapper for each pointer to member argument literal in a pair with type of the literal, where the type of the literal is discerned using `decltype` keyword. But the specialization of templates using that mechanism would be fairly verbose. 

---
