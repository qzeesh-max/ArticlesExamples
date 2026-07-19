#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace std;

//Define a type for comparator function designed to compare two instances of a class
template <typename T>
class SortComparator
{
  public:
   typedef int (*Functor)(const T&, const T&);
};

//Now define a comparator function that compares two instances of a class by a specified member generically
template <typename T, typename MemberType, MemberType T::* Member>
class SortMemberComparator
{
  public:
   static int Compare(const T& first, const T& second)
   {
     if (first.*Member==second.*Member)
       return 0;
     else if (first.*Member < second.*Member)
       return -1;
     else
       return 1;
   }
};

template<typename T>
class MultiFieldSorter
{
  protected:
   // Comparators are basically field specific functions for comparing two instances of class T 
   typedef std::vector<typename SortComparator<T>::Functor> ComparatorsList;
   ComparatorsList Comparators;

  public:
   MultiFieldSorter()
   {
   }

   ~MultiFieldSorter()
   {
   }

   void ClearFields()
   {
     Comparators.clear();
   }

   //AddComparator method is used to add a field comparator to the instance of MultiFieldSorter
   template <typename MemberType, MemberType T::* Member>
   void AddComparator()
   {
     Comparators.push_back(&SortMemberComparator<T, MemberType, Member>::Compare);
   }


   // This is what gives this class functor characteristics required by the std::sort
   // algorithm
   bool operator()(const T& first, const T& second)
   {
     for (typename ComparatorsList::iterator it=Comparators.begin(), last=Comparators.end();
      it!=last; ++it)
     {
       typename SortComparator<T>::Functor Comparator = *it;

       int CompareResult = Comparator(first, second);

       if (CompareResult < 0)
         return true;
       else if (CompareResult > 0)
         break;
     }

     return false;
   }
};


struct Person
{
public:
    std::string Firstname, Lastname, Address;
    int Age;
    double Salary, Height;

    Person(const std::string& sFirstname, 
        const std::string& sLastname, 
        const std::string& sAddress,
     int iAge, double dSalary, double dHeight) : 
            Firstname(sFirstname),
            Lastname(sLastname),
            Address(sAddress),
            Age(iAge), Salary(dSalary), Height(dHeight)
    {
    }

    std::string toString() const
    {
        std::stringstream stm;

        stm << " Firstname = " << std::setw(16) << Firstname;
        stm << "| Lastname = " << std::setw(16) << Lastname;
        stm << "| Address = " << std::setw(20) << Address;
        stm << "| Age = " << std::setw(5) << Age;
        stm << "| Salary = " << std::setw(8) << Salary;
        stm << "| Height = " << std::setw(5) << Height << std::endl;
        

        return stm.str();
    }
};

void print(const Person& p)
{
    std::cout << p.toString();
}


int main(void)
{
    std::vector<Person> Persons;

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

    std::cout << "Before Sort:" << std::endl;
    std::for_each(Persons.begin(), Persons.end(), print);
    std::cout << std::endl;

    MultiFieldSorter<Person> sorter;

    sorter.AddComparator<std::string, &Person::Lastname>();
    sorter.AddComparator<std::string, &Person::Firstname>();

    std::sort(Persons.begin(), Persons.end(), sorter);

    std::cout << "After Sort: [Lastname, Firstname]" << std::endl;
    std::for_each(Persons.begin(), Persons.end(), print);

    sorter.ClearFields();

    sorter.AddComparator<std::string, &Person::Lastname>();
    sorter.AddComparator<int, &Person::Age>();

    std::sort(Persons.begin(), Persons.end(), sorter);

    std::cout << "After Sort: [Lastname, Age]" << std::endl;
    std::for_each(Persons.begin(), Persons.end(), print);

    return 0;
}
