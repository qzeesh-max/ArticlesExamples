# C++26: #embed Directive and Compile-Time Reflection: Implementing Database Serialization

*May 16^th, 2026 By Zeeshan Qazi*

C++26 introduces two new features of interest, which are the `#embed` directive and compile-time reflection. In this article, we will show you how to put these features together to essentially generate code for serializing to/from a database using SQL schema provided to our C++ source code in a file. Our simple demonstration is just a starting point to show how one could go about using these two features to implement database applications.

For our demonstration, we have compiled our code using GCC 16.1 compiler on Ubuntu Linux. We had built our own GCC 16.1 and enabled it as a default using the alternatives feature:

```bash
$ g++ --version
g++ (GCC) 16.1.0
Copyright (C) 2026 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

In addition to this compiler we would also be using the development package for the sqlite3 to utilize a database in our demonstration.

Here is what we would be demonstrating in our demo:

- **`#embed` directive** – This directive would be used to import an SQL table creation command into a `constexpr` array, which we would parse at compile-time to extract the fields that are defined in the SQL statement.
- **`std::meta::define_aggregate` function** – This function would be used with the field definitions that are extracted from the SQL schema to form an aggregate using an incomplete type as a starting point.
- **`^^` reflection operator ** – This operator would be used to get a `std::meta::info` object for a code element such as a `struct` or a `class` and then to query its non-static data members or use it to facilitate field type definition.
- **`std::meta::nonstatic_data_members_of` function** – This function would be used for querying the non-static data members of a structure to essentially write serializers etc.
- **`[: ... :]` splice operator** – This operator would be used for splicing a component such as a field into an expression to essentially put the reflection of a field to use.
- **`template for` statement** – This statement block would be used for iterating over `constexpr` collection of fields either in their reflected form or in their field specification form.

In the interest of simplicity, we have only written a very basic parser for SQL `CREATE TABLE` statement, which essentially only handles `INT` type as the native `int` type, and treats rest of the types as `std::string`. In addition, we have some basic support for fields that accept `NULL` and treat them as `std::optional`. We have written the parser as a `constexpr` to the extent to allow usage in initializing `constexpr std::array` with the member specifications, which are later used with `std::meta::define_aggregate` method to add fields to an incomplete struct.

There are some helper functions that are used in our example to implement various features:

- `**template** &lt;**typename** S&gt; **consteval auto** getMembers()`
– This `consteval` function is used to get all the non-static data members of a struct.
- `**template** &lt;**typename** S, **typename** Lambda&gt; **void** overMembers(**const** S &amp;v, Lambda &amp;&amp;lambda)` 
`**template** &lt;**typename** S, **typename** Lambda&gt; **void** overMembers(S &amp;v, Lambda &amp;&amp;lambda)`
– This function is used to execute the functor over all the non-static data members, while passing the functor the name and value of each of those members.
- `**consteval** meta::info map_sql_type_to_cpp(**std::string_view** type_str, **bool** is_not_null)`
 – This `consteval` function is used to convert an SQL type into a C++ type. This is only a basic implementation for demonstration.
- `**template** &lt;**auto** sql_, **auto** start, **auto** end, **auto** pos, **size_t** ResultSize&gt; **consteval auto** parse_schema_to_members_helper(**const** std::array&lt;meta::info, ResultSize&gt; &amp;members)`
– This `consteval` helper function is used recursively to gather all the data members from the SQL statement`CREATE TABLE` statement in a `constexpr` collection so they may be used with `define_aggregate` to form a structure at compile-time.
`**template** &lt;auto sql_&gt; **consteval auto** parse_schema_to_members()`
– This function uses the previous helper function to convert SQL from `constexpr` string into an array of member specifications.

  

## Sample Execution

```bash
:~/mysqldemo$ ./cpp24_reflect_demo
C++26 Reflection ORM Interface (SQLite Edition)

> (list, add, lookup, remove, quit): list
{ first_name: zeeshan, last_name: qazi, address: 41 rivers edge drive, age: 45, occupation: programmer, }
{ first_name: Faizan, last_name: Qazi, address: 41 Rivers Edge Drive, age: 45, occupation: NULL, }
{ first_name: Rana, last_name: Kazi, address: 3/12 Choudhary Mohalla, Kalyan, MH, age: 66, occupation: Homemaker, }

> (list, add, lookup, remove, quit): remove
First name to remove: zeeshan
Removal query executed.

> (list, add, lookup, remove, quit): remove
First name to remove: Faizan
Removal query executed.

> (list, add, lookup, remove, quit): remove
First name to remove: Rana
Removal query executed.

> (list, add, lookup, remove, quit): list

> (list, add, lookup, remove, quit): quit
zeeshan@zeeshan:~/mysqldemo$ ./cpp26_reflect_demo
C++26 Reflection ORM Interface (SQLite Edition)

> (list, add, lookup, remove, quit): list

> (list, add, lookup, remove, quit): add
first_name: Roger
last_name: Rabbit
address (or none): none
age (0 for null): 41
occupation (or none): none
Row inserted via reflection.

> (list, add, lookup, remove, quit): add
first_name: Jessica
last_name: Rabbit
address (or none): 124 ACME drive
age (0 for null): 0
occupation (or none): PR
Row inserted via reflection.

> (list, add, lookup, remove, quit): add
first_name: Peter
last_name: Rabbit
address (or none): 123 Fake Street
age (0 for null): 0
occupation (or none): 0
Row inserted via reflection.

> (list, add, lookup, remove, quit): list
{ first_name: Roger, last_name: Rabbit, address: NULL, age: 41, occupation: NULL, }
{ first_name: Jessica, last_name: Rabbit, address: 124 ACME drive, age: NULL, occupation: PR, }
{ first_name: Peter, last_name: Rabbit, address: 123 Fake Street, age: NULL, occupation: 0, }

> (list, add, lookup, remove, quit): remove
First name to remove: Roger
Removal query executed.

> (list, add, lookup, remove, quit): list
{ first_name: Jessica, last_name: Rabbit, address: 124 ACME drive, age: NULL, occupation: PR, }
{ first_name: Peter, last_name: Rabbit, address: 123 Fake Street, age: NULL, occupation: 0, }

> (list, add, lookup, remove, quit): remove
First name to remove: Jessica
Removal query executed.

> (list, add, lookup, remove, quit): lookup
First name to search: Peter
{ first_name: Peter, last_name: Rabbit, address: 123 Fake Street, age: NULL, occupation: 0, }
```

[   schema.sql](javascript:DoLink('/download-sourcecode.php?Example=schema.sql');) 
[   cpp26_reflect_demo.cpp](javascript:DoLink('/download-sourcecode.php?Example=cpp26_reflect_demo');) 

- **G++ C++26 Compilation Instructions:** `g++ --std=c++26 -freflection ./cpp26_reflect_demo.cpp  -o cpp26_reflect_demo -lsqlite3`

---
