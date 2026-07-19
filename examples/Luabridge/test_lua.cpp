#include <typeinfo>
#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <iostream>


#include "lua_cpp_bridge.hpp"


using namespace LUA_CPP;


std::string* MyString(const std::string& text)
{
    return new std::string(text);
}

bool UseString(std::string* other)
{
    std::cout << "String = " <<  *other << std::endl;

    delete other;

    return true;
}


int sayHi(std::string& yo, int i, LuaFunctionWrapper w)
{
    std::cout << "Hi " << yo << " some integer " << i << std::endl;

    std::cout << (int)w(yo, i) << std::endl;

    return 15;
}

class Vector
{
private:
    int x, y, z;

public:
    Vector(int x, int y, int z) : x(x), y(y), z(z)
    {
    }

    ~Vector()
    {
        std::cout << "Vector collected -";

        Print();
    }

    void SetX(int x) {
        this->x = x;
    }

    void SetY(int y) {
        this->y = y;
    }

    void SetZ(int z) {
        this->z = z;
    }


    void GetCoords(int& x, int& y, int& z)
    {
        x = this->x;
        y = this->y;
        z = this->z;
    }


    void Print()
    {
        std::cout << " this = " << (void*)this << " x = " << x << " y = " << y << " z = " << z << std::endl;
    }


    void Set(boost::shared_ptr<Vector> other)
    {
        x = other->GetX();
        y = other->GetY();
        z = other->GetZ();
    }


    int GetX() {
        return x;
    }
    int GetY() {
        return y;
    }
    int GetZ() {
        return z;
    }
};

void throwTestException(int count)
{
	std::cout << "received count as argument " << count << std::endl;
	throw std::logic_error("Exception from C++");
}

int main (void) {


    char buff[256];
    int error;
    lua_State *L = luaL_newstate();   /* opens Lua */



    luaL_openlibs(L);


    RegisterLuaClass<Vector, int, int, int>(L, "Vector",
                                            LUA_CLASS_METHODS
                                            LUA_REGISTER_METHOD(Vector,SetX)
                                            LUA_REGISTER_METHOD(Vector,SetY)
                                            LUA_REGISTER_METHOD(Vector,SetZ)
                                            LUA_REGISTER_METHOD(Vector,Set)
                                            LUA_REGISTER_METHOD(Vector,GetCoords)
                                            LUA_REGISTER_METHOD(Vector,Print)
                                            LUA_REGISTER_METHOD(Vector,GetX)
                                            LUA_REGISTER_METHOD(Vector,GetY)
                                            LUA_REGISTER_METHOD(Vector,GetZ)
                                            LUA_END_CLASS_METHODS(Vector));


    RegisterLuaFunction(L, "yo", sayHi);
    RegisterLuaFunction(L, "MyString", MyString);
    RegisterLuaFunction(L, "UseString", UseString);
    RegisterLuaFunction(L, "throwTestException", throwTestException);

    std::string luaChunk;
#ifdef __EMSCRIPTEN__
    freopen("/test.lua", "r", stdin);
#endif
    while (fgets(buff, sizeof(buff), stdin) != NULL) {

        luaChunk+=buff;

    }

    error = luaL_loadbuffer(L, luaChunk.c_str(), luaChunk.length(), "line") ||
            lua_pcall(L, 0, 0, 0);
    if (error) {
        fprintf(stderr, "Error in LUA: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);  /* pop error message from the stack */
    }

    lua_close(L);
    return 0;
}

