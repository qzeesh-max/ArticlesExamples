# Using Pointers to Members as Template Arguments

*May 19^th, 2012 By Zeeshan Qazi*

Templates in C++ provide very valuable functionality by giving the programmer ability to create generic classes and functions that may be specialized to work with specific classes, and even literal constants. The C++ templates are also extremely helpful in being able to define generic programming frameworks like STL and Boost. These templates can also be utilized for advanced metaprogramming like the ability provided by Boost's MPL. But today we will learn about lesser known ability of C++ templates to allow specifying pointer to class members as template arguments.

C++ not only supports ability to create pointers to static members, local variables, parameters and non-member functions, but it also supports the ability to create pointers to member variables and member functions while preserving the type information about the member containing class. This is very helpful in being able to invoke member functions dynamically or accessing member fields dynamically. This ability is very useful for implementing a variety of mechanisms very efficiently.

### Member Dereference Operators

C++ has two operators that can be used to dereference pointer to members namely, `.*` operator, and `-&gt;*` operator.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=ptm-ex1');)
   Compilation Instructions:  g++ ptm-ex1.cpp -o ptm-ex1

**Example 1:** Pointer to Member Reference / Dereference

`**struct** SampleStruct
{
 **private:**
  string Name;

 **public:**
  SampleStruct(**const** string& sName): Name(sName) { } 

  **void** printHello()
  {
    cout &lt;&lt; "Hello " &lt;&lt; Name &lt;&lt; endl; 
  }

  **void** printBye()
  {
    cout &lt;&lt; "Bye " &lt;&lt; Name &lt;&lt; endl; 
  }

  **void** printBadCommand()
  {
    cout &lt;&lt; "Bad Command!" &lt;&lt; endl; 
  }
};

**int** main()
{
  string Name;

  cout &lt;&lt; "Your name:";
  cin &gt;&gt; Name;

  SampleStruct ss(Name);

  **void** (SampleStruct::* function)();
  **char** ch;

  cout &lt;&lt; "Please choose action:" &lt;&lt; endl &lt;&lt; endl;
  cout &lt;&lt; "[H] Hello" &lt;&lt; endl;
  cout &lt;&lt; "[B] Bye" &lt;&lt; endl &lt;&lt; endl;

  cin &gt;&gt; ch;

  **switch** (ch)
  {
    **case** 'H':
    **case** 'h':
      function = &SampleStruct::printHello;
      **break**;
    **case** 'B':
    **case** 'b':
      function = &SampleStruct::printBye;
      **break**;
    **default**:
      function = &SampleStruct::printBadCommand;
      **break**;
  }

  // Invoke the member function on a particular instance by using **.* operator** to get the function to **bind** to the instance of the class.
  (ss.*function)();

  **return** 0;
}
`

### Pointer to Member Template Arguments and Implementing a Dynamic Compare Functor

Now that we are familiar with pointer to members and how to utilize them, let us move on to their use in templates. For our example we will be implementing a simple mechanism for sorting a structure by its various members, more or less programmatically at runtime, while minimizing the code needed to perform this task.We will be utilizing `std::sort` from STL `algorithms` header file, and we will be providing our own comparator functor that will allow us to dynamically choose multiple fields in structure in order for sorting by them. This is a very simple example meant only to demonstrate how to use pointer to member fields as template arguments.

Firstly, we will define a comparator function designed to compare two instances of a class or structure by a specific field. The purpose of defining this function is to provide generic ability to compare to structures by a specific field. We will be definining this function as a static function inside a separate class solely for the purpose of making the code easy to read.

**Example 2:** Defining a Comparator that sorts by a member field

`//Define a type for comparator function designed to compare two instances of a class
**template** &lt;**typename** T&gt;
**class** SortComparator
{
  **public**:
   **typedef int** (*Functor)(const T&, const T&);
};

//Now define a comparator function that compares two instances of a class by a specified member generically
**template** &lt;**typename** T, **typename** MemberType, MemberType T::* Member&gt;
**class** SortMemberComparator
{
  **public**:
   **static int** Compare(**const** T& first, **const** T& second)
   {
     **if** (first.*Member==second.*Member)
       **return** 0;
     **else if** (first.*Member // **Comparators** are basically field specific functions for comparing two instances of class **T** 
   **typedef** std::vector&lt;**typename** SortComparator&lt;T&gt;::Functor&gt; ComparatorsList;
   ComparatorsList Comparators;

  **public**:
   MultiFieldSorter()
   {
   }

   ~MultiFieldSorter()
   {
   }

   **void** ClearFields()
   {
     Comparators.clear();
   }

   //**AddComparator** method is used to add a field comparator to the instance of **MultiFieldSorter**
   **template** &lt;**typename** MemberType, MemberType T::* Member&gt;
   **void** AddComparator()
   {
     Comparators.push_back(&amp;SortMemberComparator&lt;T, MemberType, Member&gt;::Compare);
   }

   // This is what gives this class functor characteristics required by the **std::sort**
   // algorithm
   **bool** operator()(**const** T& first, **const** T& second)
   {
     **for** (**typename** ComparatorsList::iterator it=Comparators.begin(), last=Comparators.end();
      it!=last; ++it)
     {
       **typename** SortComparator&lt;T&gt;::Functor Comparator = *it;

       **int** CompareResult = Comparator(first, second);

       **if** (CompareResult  0)
         **break**;
     }

     **return false**;
   }
};
`

Now let us use this class in our demonstration to show how it works. Our example will demonstrate how we can programmatically tell our instance of sorter to choose fields to sort by.

[   Download](javascript:DoLink('/download-sourcecode.php?Example=ptm-ex2');)
   Compilation Instructions:  g++ ptm-ex2.cpp -o ptm-ex2

**Example 4:** Using **MultiFieldSorter** in our example

`**struct** Person
{
**public**:
    std::string Firstname, Lastname, Address;
    **int** Age;
    **double** Salary, Height;

    Person(**const** std::string& sFirstname, 
        **const** std::string& sLastname, 
        **const** std::string& sAddress,
            **int** iAge, **double** dSalary, **double** dHeight) : 
            Firstname(sFirstname),
            Lastname(sLastname),
            Address(sAddress),
            Age(iAge), Salary(dSalary), Height(dHeight)
    {
    }

    std::string toString() **const**
    {
        std::stringstream stm;

        stm &lt;&lt; " Firstname = " &lt;&lt; std::setw(16) &lt;&lt; Firstname;
        stm &lt;&lt; "| Lastname = " &lt;&lt; std::setw(16) &lt;&lt; Lastname;
        stm &lt;&lt; "| Address = " &lt;&lt; std::setw(20) &lt;&lt; Address;
        stm &lt;&lt; "| Age = " &lt;&lt; std::setw(5) &lt;&lt; Age;
        stm &lt;&lt; "| Salary = " &lt;&lt; std::setw(8) &lt;&lt; Salary;
        stm &lt;&lt; "| Height = " &lt;&lt; std::setw(5) &lt;&lt; Height &lt;&lt; std::endl;
        

        **return** stm.str();
    }
};

**void** print(**const** Person& p)
{
    std::cout &lt;&lt; p.toString();
}

**int** main(**void**)
{
    std::vector&lt;Person&gt; Persons;

    Persons.push_back(Person("James", "Havenschroff", "123 Phoney Avenue", 55, 10000, 5.5));
    Persons.push_back(Person("Carlos", "Havenschroff", "123 Phoney Avenue", 33, 4000, 3.5));
    Persons.push_back(Person("Baikos", "Maduschroff", "44 Chariot Way Blvd", 35, 23000, 4.5));
    Persons.push_back(Person("Lath", "Homerburg", "96 Charred Tuna Way", 45, 8000, 6.5));
    Persons.push_back(Person("Santos", "Koltamaru", "888 Penguins Fuss", 27, 7000, 4.7));
    Persons.push_back(Person("Chole", "Saga", "10242 Yarod Way", 85, 0, 7.5));
    Persons.push_back(Person("Deschel", "Halbard", "95 Spartan Avenue", 36, 4000, 7.5));
    Persons.push_back(Person("Cohen", "Talschroff", "40 Baridi Way", 33, 5300, 6.1));
    Persons.push_back(Person("Cohen", "Maigard", "40 Baridi Way", 55, 10000, 5.5));
    Persons.push_back(Person("Fantos", "Shakahara", "90 Broadwalk Avenue", 55, 10000, 5.5));
    Persons.push_back(Person("Laprischel", "Havenschroff", "123 Phoney Avenue", 51, 10000, 5.5));

    std::cout &lt;&lt; "Before Sort:" &lt;&lt; std::endl;
    std::for_each(Persons.begin(), Persons.end(), print);
    std::cout &lt;&lt; std::endl;

    MultiFieldSorter&lt;Person&gt; sorter;

    sorter.AddComparator&lt;std::string, &Person::Lastname&gt;();
    sorter.AddComparator&lt;std::string, &Person::Firstname&gt;();

    std::sort(Persons.begin(), Persons.end(), sorter);

    std::cout &lt;&lt; "After Sort: [Lastname, Firstname]" &lt;&lt; std::endl;
    std::for_each(Persons.begin(), Persons.end(), print);

    sorter.ClearFields();

    sorter.AddComparator&lt;std::string, &Person::Lastname&gt;();
    sorter.AddComparator&lt;**int**, &Person::Age&gt;();

    std::sort(Persons.begin(), Persons.end(), sorter);

    std::cout &lt;&lt; "After Sort: [Lastname, Age]" &lt;&lt; std::endl;
    std::for_each(Persons.begin(), Persons.end(), print);

    **return** 0;
}
`

As you can see from the above example we have created a dynamic mechanism for setting up a Compare functor for the `std::sort` algorithm using template arguments with pointers to members. The usefulness of this mechanism in allowing us to write concise code is apparent from the above example. This same mechanism can be utilized for writing many other complex algorithms quite concisely.

---
