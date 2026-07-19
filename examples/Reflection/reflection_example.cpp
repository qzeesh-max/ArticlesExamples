#include <string>
#include <map>
#include <list>
#include <vector>
#include <set>
#include <iostream>

#include "reflection.hpp"


class Vector
{
protected:
    int x, y, z;
public:
    Vector ():
        x (0), y (0), z (0) { }
    Vector (int x, int y, int z):
        x (x), y (y), z (z)
    {
    }
    int getX () const
    {
        return x;
    }
    int getY () const
    {
        return y;
    }
    int getZ () const
    {
        return z;
    }

    void _getX(int& i)
    {
        i = x;
    }

    bool _getXYZ(int& i, int& j, int& k)
    {
        i = x;
        j = y;
        k = z;

	return true;
    }


    void setX (int i)
    {
        x = i;
    }
    void setY (int i)
    {
        y = i;
    }
    void setZ (int i)
    {
        z = i;
    }
};
int main ()
{
    Module TestModule ("TestModule");
    export_module (TestModule)
    {
        insert_class (Vector)
        {
	    // a constructor with particular types of parameters
            __constructor (int, int, int);

	    // default constructor
	    __defaultconstructor ();

	    // methods that return values and are const
            __method (getX);
            __method (_getX);
            __method (getY);
            __method (getZ);

	    // method that has reference arguments
            __method (_getXYZ);

	    // naming an argument
            __method (setX, "X");

	    // methods that take arguments
            __method (setY);
            __method (setZ);
        }

	typedef Vector SecondVector;

	insert_class (SecondVector)
	{
	    // a constructor with particular types of parameters
            __constructor (int, int, int);

	    // default constructor
	    __defaultconstructor ();

	    // method that has reference arguments
            __method (_getXYZ);		
	}
    }

    Parameters_t p;
    boost::any ret;

    p.push_back(12);
    p.push_back(23);
    p.push_back(26);

    auto kapa = TestModule.getClass ("Vector")->construct(p);

    p.clear();

    auto defkapa = TestModule.getClass ("SecondVector")->construct(p);


    TestModule.getClass ("Vector")->getMethod ("getX")-> call (kapa, ret, p);
    std::cout << boost::any_cast < int >(ret) << std::endl;
    TestModule.getClass ("Vector")->getMethod ("getY")-> call (kapa, ret, p);
    std::cout << boost::any_cast < int >(ret) << std::endl;


    p.push_back(12);

    TestModule.getClass ("Vector")->getMethod ("setY")-> call (kapa, p);


    TestModule.getClass ("Vector")->getMethod ("setZ")-> call (kapa, p);

    std::cout << "getY:" << ((Vector*)kapa)->getY() << std::endl;
    std::cout << "getZ:" << ((Vector*)kapa)->getZ() << std::endl;

    p.clear();
    TestModule.getClass ("Vector")->getMethod ("getY")-> call (kapa, ret, p);
    std::cout << boost::any_cast < int >(ret) << std::endl;


    TestModule.getClass ("Vector")->getMethod ("getZ")-> call (kapa, ret, p);
    std::cout << boost::any_cast < int >(ret) << std::endl;

    int x, y;
    int z = 12;

    p.push_back(boost::ref(x));

    TestModule.getClass ("Vector")->getMethod ("_getX")-> call (kapa, p);

    std::cout << "X= " << x << std::endl;

    p.push_back(boost::ref(y));
    p.push_back(boost::ref(z));

    TestModule.getClass ("Vector")->getMethod ("_getXYZ")->call (kapa, p);

    std::cout << "X= " << x << ", Y= " << y << ", Z=" << z << std::endl;

    p.clear();

    const int al = 112;

    p.push_back(al);

    TestModule.getClass ("Vector")->getMethod ("setX")-> call (kapa, p);

    p.clear();

    p.push_back(boost::ref(x));
    p.push_back(boost::ref(y));
    p.push_back(boost::ref(z));

    TestModule.getClass ("Vector")->getMethod ("_getXYZ")->call (kapa, p);

    std::cout << "X= " << x << ", Y= " << y << ", Z=" << z << std::endl;

    TestModule.getClass ("SecondVector")->getMethod ("_getXYZ")->call (kapa, p);

    std::cout << "X= " << x << ", Y= " << y << ", Z=" << z << std::endl;

    TestModule.getClass ("Vector")->destroy(kapa);
    TestModule.getClass ("SecondVector")->destroy(defkapa);

    return 0;
}
