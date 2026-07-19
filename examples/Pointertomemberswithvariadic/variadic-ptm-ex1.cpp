#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

using namespace std;

template<typename ...arguments>
class ascending_compare;

template<>
class ascending_compare<>;

template<typename first_t>
class ascending_compare<first_t>
{
    private:
        const first_t first;

    public:
        constexpr ascending_compare(const first_t first) : first(first) {}

        template <typename T>
        bool operator()(const T& a, const T& b) const
        {
            return a.*first < b.*first;
        }
};

template<typename first_t, typename second_t>
class ascending_compare<first_t, second_t>
{
    private:
        const first_t first;
        const second_t second;

    public:
        constexpr ascending_compare(const first_t first, const second_t second) : first(first), second(second) {}

        template <typename T>
        bool operator()(const T& a, const T& b) const 
        {
            return  (a.*first < b.*first)||
                ((a.*first==b.*first) && (a.*second < b.*second));
        }
};

template<typename first_t, typename second_t, typename ...others_t>
class ascending_compare<first_t, second_t, others_t...>
{
    private:
        const first_t first;
        const second_t second;
        const ascending_compare<others_t...> others;

    public:
        constexpr ascending_compare(const first_t first, const second_t second, const others_t... others) : first(first), second(second), others(others...) {}

        template <typename T>
        bool operator()(const T& a, const T& b) const 
        {
            return (a.*first<b.*first) ||
                ((a.*first==b.*first) && ((a.*second < b.*second) || 
                ((a.*second==b.*second) && others(a,b))));
        }
};



template<typename ...T>
constexpr ascending_compare<T...> Ascending_Compare(const T...  argument)
{
    return ascending_compare<T...>(argument...);
}




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
    vector<Test> Tests;

    for (int i = 0; i <5000; i++)
        Tests.push_back(Test(i/10, (5000-i)/5 , i/50, rand()/3.92));

    sort(Tests.begin(), Tests.end(), Ascending_Compare(&Test::a, &Test::b, &Test::c, &Test::d));

    for (int i = 0; i <5000; i++)
        cout << "a = " << Tests[i].a << ", b = " << Tests[i].b << ", c = " << Tests[i].c << ", d = " << std::setprecision(3) << fixed << Tests[i].d << endl;

    
}
