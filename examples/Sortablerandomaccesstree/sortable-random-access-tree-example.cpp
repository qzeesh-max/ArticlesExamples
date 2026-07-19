#include "sortable-random-access-tree.hpp"

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



void print(SortableRandomAccessTree<Person> & People)
{
    int i = 0;
    
    for (SortableRandomAccessTree<Person>::Iterator it = People.begin(), end = People.end(); it!=end; ++it)
    {
    	// dump and confirm index
    	std::cout << i << ": " << (*it).toString() << " -- " << ((&*People.GetByIndex(i) == &*it) ? "good" : "bad")  << std::endl;
	
	i++;
    }
    


}
 
int main(void)
{
    DynamicComparatorBuilder<Person> Comparator;
    
    Comparator.AddField<int, &Person::Age>(AscendingSort_t());
    Comparator.AddField<std::string, &Person::Lastname>(DescendingSort_t());
    
    SortableRandomAccessTree<Person> People(Comparator);
        
    
    People.Insert(Person("James", "Havenschroff", "123 Phoney Avenue", 55, 10000, 5.5));
    People.Insert(Person("Carlos", "Havenschroff", "123 Phoney Avenue", 33, 4000, 3.5));
    People.Insert(Person("Baikos", "Maduschroff", "44 Chariot Way Blvd", 35, 23000, 4.5));
    People.Insert(Person("Lath", "Homerburg", "96 Charred Tuna Way", 45, 8000, 6.5));
    People.Insert(Person("Santos", "Koltamaru", "888 Penguins Fuss", 27, 7000, 4.7));
    People.Insert(Person("Chole", "Saga", "10242 Yarod Way", 85, 0, 7.5));
    People.Insert(Person("Deschel", "Halbard", "95 Spartan Avenue", 36, 4000, 7.5));
    People.Insert(Person("Cohen", "Talschroff", "40 Baridi Way", 33, 5300, 6.1));
    People.Insert(Person("Cohen", "Maigard", "40 Baridi Way", 55, 10000, 5.5));
    People.Insert(Person("Fantos", "Shakahara", "90 Broadwalk Avenue", 55, 10000, 5.5));
    People.Insert(Person("Laprischel", "Havenschroff", "123 Phoney Avenue", 51, 10000, 5.5));
  
    std::cout << "Sorted by Age in Asecending, LastName in Descending" << std::endl << std::endl;
    
    print(People);    
    
    int index;

    
    People.FindWithIndex(index, 27, std::string("Koltamaru"));
    
    std::cout << "Find age 27, lastname Koltamaru : index = " << index << std::endl;
    
    People.FindWithIndex(index, 33, std::string("Talschroff"));
    
    std::cout << "Find age 40, lastname Talschroff : index = " << index << std::endl;
    
    People.FindWithIndex(index, 55, std::string("Maigard"));
    
    std::cout << "Find age 55, lastname Maigard : index = " << index << std::endl;
    
    
    std::cout << "Sorted by LastName in Descending, Age Ascending" << std::endl << std::endl;
    
    Comparator.Clear();
    
    Comparator.AddField<std::string, &Person::Lastname>(DescendingSort_t());
    Comparator.AddField<int, &Person::Age>(AscendingSort_t());
    
    People.Resort(Comparator);
    
    print(People);    
    
    People.FindWithIndex(index, std::string("Koltamaru"), 27);
    
    std::cout << "Find age 27, lastname Koltamaru : index = " << index << std::endl;
    
    People.FindWithIndex(index, std::string("Talschroff"), 33);
    
    std::cout << "Find age 40, lastname Talschroff : index = " << index << std::endl;
    
    People.FindWithIndex(index, std::string("Maigard"), 55);
    
    std::cout << "Find age 55, lastname Maigard : index = " << index << std::endl;

    
    
    std::cout << "Sorted by LastName in Ascending, FirstName in Ascending" << std::endl << std::endl;
    
    Comparator.Clear();
    
    Comparator.AddField<std::string, &Person::Lastname>(AscendingSort_t());
    Comparator.AddField<std::string, &Person::Firstname>(AscendingSort_t());
    
    People.Resort(Comparator);

    
    print(People);    
    
       
    std::cout << "Sorted by Age in Ascending" << std::endl << std::endl;
    
    Comparator.Clear();
    
    Comparator.AddField<int, &Person::Age>(AscendingSort_t());
    
    People.Resort(Comparator);

    
    print(People);    
    

    
    return 0;
}
