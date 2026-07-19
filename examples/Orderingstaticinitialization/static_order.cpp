#include <new>


#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include "ordered_static_initializer.hpp"

class output
{
	public:
		output()
		{
            std::ios_base::Init init; // Ensure std::cout is initialized
		}

		std::ostream& operator()()
		{
			return std::cout;
		}

};

ORDERED_NONMEM_STATIC_DECL(output, out);



class printer
{
	private:
		std::string text;

	public:
		printer(const char* text) : text(text)
		{
			out() << "construct: " << text << std::endl;
		}

		printer(const std::string& text) : text(text)
		{
			out() << "construct: " << text << std::endl;
		}

		operator const std::string&()
		{
			return text;
		}

		~printer()
		{
			out() << "destroy: " << text << std::endl;			
		}


};


class printer2 {
public:
	ORDERED_MEM_STATIC_DECL(printer,printer2,one);
	ORDERED_MEM_STATIC_DECL(printer,printer2,two);
};

ORDERED_NONMEM_STATIC_DECL(printer, non_member);


ORDERED_MEM_STATIC(printer,printer2,one, "printer2::Static one") STATIC_REQUIRES_MEM(printer2,two) STATIC_REQUIRES_NONMEM(non_member) STATIC_REQUIRES_NONMEM(out);

std::ostream& operator<<(std::ostream& o, const printer& p)
{
	return o << (const std::string&)p;
}


ORDERED_NONMEM_STATIC(int, myInt,12);
ORDERED_NONMEM_STATIC(printer, non_member2, "This should initialize second, because it depends on first") STATIC_REQUIRES_NONMEM(non_member) STATIC_REQUIRES_NONMEM(out);
ORDERED_NONMEM_STATIC(printer, non_member, "This should initialize first, because second depends on it") STATIC_REQUIRES_MEM(printer2, two) STATIC_REQUIRES_NONMEM(out);
ORDERED_MEM_STATIC(printer,printer2,two, "printer2::Static two") STATIC_REQUIRES_NONMEM(out);

ORDERED_NONMEM_STATIC(output, out);


int main (void)
{
	out() << "myInt = " << myInt << " non_member2 = " << non_member2 << "\t" << "non_member = " << non_member << std::endl;
}
