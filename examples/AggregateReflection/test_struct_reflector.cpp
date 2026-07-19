#include <iostream>
#include <string>
#include "struct_reflector.hpp"

struct Person {
	std::string name;
	int age;
};

int main()
{
	std::cout << "Fields: " << std::endl;

	for (auto a: get_field_names<Person>())
	{
		std::cout << "\t" << a << std::endl;
	}

	std::cout << "Fields + Values: " << std::endl;

	for_each_member(Person{"Zeeshan", 45}, [](const auto& name, const auto& value) { std::cout << "\t" << name << " = " << value << std::endl; });

	std::cout << "Just Values: " << std::endl;

	for_each_member_value(Person{"Zeeshan", 45}, [i = 0](const auto& value) mutable { std::cout << "\t[" << (i++) << "] = " << value << std::endl; });
}
