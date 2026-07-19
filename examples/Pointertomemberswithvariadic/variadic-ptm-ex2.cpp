#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <stdexcept>
#include <algorithm>

using namespace std;

template<typename ...argumentTypes>
class ac_type_wrapper
{
public:
    template<argumentTypes ...arguments>
    class ascending_compare;
};


template<>
template<>
class ac_type_wrapper<>::ascending_compare <>;


template<typename first_t>
class ac_type_wrapper<first_t>
{
public:
    template<first_t first>
    class ascending_compare
    {
        public:
            template <typename T>
            bool operator()(const T& a, const T& b) const
            {
                    return a.*first < b.*first;
            }
    };
};



template<typename first_t, typename second_t>
class ac_type_wrapper<first_t, second_t>
{
public:
    template<first_t first, second_t second>
    class ascending_compare
    {
        public:
            template <typename T>
            bool operator()(const T& a, const T& b) const 
            {
                return  (a.*first < b.*first)||
                    ((a.*first==b.*first) && (a.*second < b.*second));
            }
    };
};

template<typename first_t, typename second_t, typename ...others_t>
class ac_type_wrapper<first_t, second_t, others_t...>
{
public:
    template<first_t first, second_t second, others_t...others>
    class ascending_compare
    {
        public:
            template <typename T>
            bool operator()(const T& a, const T& b) const 
            {
                typedef typename ac_type_wrapper<others_t...>::template ascending_compare<others...> __others;

                return (a.*first<b.*first) ||
                    ((a.*first==b.*first) && ((a.*second < b.*second) || 
                    ((a.*second==b.*second) && 
                     __others()(a,b))));
            }
    };
};


template<typename ...T>
ac_type_wrapper<T...> __Ascending_Compare(const T...  argument) 
{
    throw exception("__Ascending_Compare is not meant to be called, use Ascending_Compare macro with pointer to member literals");
}

#define Ascending_Compare(...) decltype(__Ascending_Compare(__VA_ARGS__))::ascending_compare<__VA_ARGS__>

struct Test
{ 
    int a;
    short b;
    float c;
    double d; 

    Test(int a, short b, float c, double d): a(a), b(b), c(c), d(d){}
};

int main(void)
{
    typedef set<Test, Ascending_Compare(&Test::a, &Test::b, &Test::c, &Test::d)> TestsSet; 
    TestsSet Tests2;

    for (int i = 0; i <5000; i++)
        Tests2.insert(Test(i/10, (5000-i)/5 , i/50, random()/3.92));


    for (TestsSet::const_iterator it = Tests2.begin(), end = Tests2.end(); it!=end; ++it)
        cout << "a = " << it->a << ", b = " << it->b << ", c = " << it->c << ", d = " << std::setprecision(3) << fixed << it->d << endl;

}
