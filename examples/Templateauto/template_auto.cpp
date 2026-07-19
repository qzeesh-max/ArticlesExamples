#include "template_auto.hpp"
#include <string>
#include <iostream>
#include <iomanip>
#include <cstdint>

using namespace std;

class TestBase
{
protected:
    int WordField{15};

};


class TestSimple : public TestBase
{
    template <typename T = TestSimple>
    using FIELD_APPLICATOR = FieldApplicator<&T::IntField, &T::CharField, &T::StringField, &TestBase::WordField>;

    int IntField {11};
    char CharField {120};
    std::string StringField{"Hello"};


public:
    TestSimple()
    {
    }

    template <typename Stream>
    void read(Stream& s)
    {
        FIELD_APPLICATOR<>::read(s, *this);
    }

    template <typename Stream>
    void write(Stream& s) const
    {
        FIELD_APPLICATOR<>::write(s, *this);
    }

    bool operator==(const TestSimple& o) const
    {
        return FIELD_APPLICATOR<>::apply_as_tuple([](const auto& a, const auto& b) {
            return a == b;
        }, *this, o);
    }

    bool operator!=(const TestSimple& o) const
    {
        return FIELD_APPLICATOR<>::apply_as_tuple([](const auto& a, const auto& b) {
            return a != b;
        }, *this, o);
    }

    bool operator<(const TestSimple& o) const
    {
        return FIELD_APPLICATOR<>::apply_as_tuple([](const auto& a, const auto& b) {
            return a < b;
        }, *this, o);
    }

    bool operator>(const TestSimple& o) const
    {
        return FIELD_APPLICATOR<>::apply_as_tuple([](const auto& a, const auto& b) {
            return a > b;
        }, *this, o);
    }



    ~TestSimple()
    {
    }
};

struct strwriter
{
    template <typename Object, typename T>
    strwriter& read(Object &obj, T& o)
    {
	std::cout << "\n read field @ 0x" << std::hex << std::setfill('0') 
		  << std::setw(sizeof(uintptr_t)*2) 
                  << (reinterpret_cast<const char*>(&o)-reinterpret_cast<const char*>(&obj)) << ":";
        std::cin >> o;

        return *this;
    }

    template <typename Object, typename T>
    strwriter& write(const Object& obj, const T& o)
    {
	std::cout << "\n write field @ 0x" << std::hex << std::setfill('0') 
		  << std::setw(sizeof(uintptr_t)*2) 
		  << (reinterpret_cast<const char*>(&o)-reinterpret_cast<const char*>(&obj)) << ":";
        std::cout << std::dec << o;

        return *this;
    }
} wr;


int main()
{
    TestSimple DataSet, DataSet2;

    DataSet.write(wr);

    DataSet.read(wr);

    DataSet.write(wr);
    

    std::cout << std::endl << std::boolalpha << "Equality : " << (DataSet == DataSet2) << std::endl;
    std::cout << std::endl << std::boolalpha << "Inequality : " << (DataSet != DataSet2) << std::endl;
    std::cout << std::endl << std::boolalpha << "Less: " << (DataSet < DataSet2) << std::endl;
    std::cout << std::endl << std::boolalpha << "Greater : " << (DataSet > DataSet2) << std::endl;


    return 0;
}
