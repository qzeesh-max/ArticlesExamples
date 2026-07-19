#pragma once

#include <typeinfo>
#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <map>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}


namespace LUA_CPP
{

typedef void (*MethodRegistryFunction)(lua_State *L);


// this class detects a reference to ensure that the caller is informed of additional return values!
template <typename T> class reference_detector
{
private:
    T value;
    bool wasReferenced;
    int& returnValueCount;
    lua_State *luaState;


public:
    reference_detector(const T& value, int& returnValueCount, lua_State *luaState):
        value(value), wasReferenced(false), returnValueCount(returnValueCount), luaState(luaState)
    {
    }

    reference_detector(reference_detector<T>&& other) : returnValueCount(other.returnValueCount)
    {
        value = other.value;
        other.value = T();
        wasReferenced = other.wasReferenced;
        other.wasReferenced = false;
        luaState = other.luaState;
        other.luaState=nullptr;
    }

    operator T() const
    {
        return value;
    }

    operator const T&() const
    {
        return value;
    }

    operator T&()
    {
        wasReferenced = true;
        returnValueCount++;
        return value;
    }

    ~reference_detector();
};


template <typename T> class lua_choose_type
{
    static void Push(lua_State *L, T data)
    {
        static_assert(!sizeof(T), "Only primitive types, pointers to classes / structs, LUA functions or shared pointers to classes/structs may be pushed to LUA");
    }

    static bool IsAtAddress(lua_State *L, int index)
    {
        static_assert(!sizeof(T), "Only primitive types, pointers to classes / structs, LUA functions or shared pointers to classes/structs may be retrieved from LUA");
    }


    static reference_detector<T> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        static_assert(!sizeof(T), "Only primitive types, pointers to classes / structs, LUA functions or shared pointers to classes/structs may be retrieved from LUA");
    }

};

template <typename T> class lua_choose_type<T*>
{
public:
    static void Push(lua_State *L, T* data)
    {
        lua_createtable(L, 2, 2);
        lua_pushlightuserdata(L, reinterpret_cast<void*>(data));
        lua_setfield(L, -2, "__cpp_data");
        lua_pushlightuserdata(L,
                              const_cast<void*>(reinterpret_cast<const void*>(&typeid(data))));
        lua_setfield(L, -2, "__cpp_type");
    }

    static bool IsAtAddress(lua_State *L, int index)
    {
        if (lua_istable(L, index))
        {
            lua_pushstring(L, "__cpp_data");
            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                return false;

            lua_pop(L, 1);

            lua_pushstring(L, "__cpp_type");

            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                return false;

            bool hasSameType = &typeid(T) == lua_touserdata(L, -1);

            lua_pop(L, 1);

            return hasSameType;
        }

        return false;
    }

    static reference_detector<T*> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (lua_istable(L, index))
        {
            lua_pushstring(L, "__cpp_data");
            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            void * value = lua_touserdata(L, -1);
            lua_pop(L, 1);

            lua_pushstring(L, "__cpp_type");

            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            bool hasSameType = &typeid(T*) == lua_touserdata(L, -1);

            lua_pop(L, 1);

            if (hasSameType)
                return reference_detector<T*>(reinterpret_cast<T*>(value), returnValueCount, L);


            throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


        } else if (lua_isnil(L, index))
        {
            return reference_detector<T*>(nullptr, returnValueCount, L);
        }

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");
    }
};



template <typename T>
class lua_choose_type<boost::shared_ptr<T>& >
{
private:
    static std::map<void*, boost::shared_ptr<T> > instances;
    static int instanceTableReference;
public:
    static bool FindAndPushTable(lua_State *L, void * data)
    {
        if (instanceTableReference < 0)
        {
            lua_createtable(L, 8, 8);
            lua_createtable(L, 1, 1);
            lua_pushstring(L, "v");
            lua_setfield(L, -2, "__mode");
            lua_setmetatable(L, -2);

            instanceTableReference = luaL_ref(L, LUA_REGISTRYINDEX);

            return false;
        } else {
            std::stringstream stm;

            stm << data;

            lua_rawgeti(L, LUA_REGISTRYINDEX, instanceTableReference);

            lua_getfield(L, -1, stm.str().c_str());

            lua_remove(L, -2);

            if (!lua_isnil(L, -1))
            {
                return true;
            }

            lua_pop(L, 1);

            return false;

        }
    }

    static int DestroyObject(lua_State *L)
    {
        if (lua_istable(L, 1))
        {
            lua_pushstring(L, "__cpp_data");
            lua_gettable(L, 1);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            void * value = lua_touserdata(L, -1);
            lua_pop(L, 1);

            lua_pushstring(L, "__cpp_type");

            lua_gettable(L, 1);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            bool hasSameType = &typeid(boost::shared_ptr<T>*) == lua_touserdata(L, -1);

            lua_pop(L, 1);

            if (hasSameType)
            {
                instances.erase(reinterpret_cast<boost::shared_ptr<T>*>(value)->get());

                return 0;
            }

            throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


        }

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

    }

    static void Push(lua_State *L, const boost::shared_ptr<T>& data)
    {
        if (!FindAndPushTable(L, data.get()))
        {
            boost::shared_ptr<T> * sharedPtr;

            auto it = instances.find(data.get());

            if (it!=instances.end())
                sharedPtr = &it->second;
            else
                sharedPtr = &(instances.emplace(data.get(), data).first->second);


            lua_createtable(L, 2, 2);
            lua_pushlightuserdata(L, reinterpret_cast<void*>(sharedPtr));
            lua_setfield(L, -2, "__cpp_data");
            lua_pushlightuserdata(L,
                                  const_cast<void*>(reinterpret_cast<const void*>(&typeid(sharedPtr))));
            lua_setfield(L, -2, "__cpp_type");


            lua_rawgeti(L, LUA_REGISTRYINDEX, instanceTableReference);

            lua_pushvalue(L, -2);

            std::stringstream stm;

            stm << data.get();

            lua_setfield(L, -2, stm.str().c_str());

            lua_pop(L, 1);

            // meta-table
            lua_createtable(L, 2, 2);
            lua_pushcfunction(L, &DestroyObject);
            lua_setfield(L, -2, "__gc");
            lua_setmetatable(L, -2);



        }

    }

    static bool IsAtAddress(lua_State *L, int index)
    {
        if (lua_istable(L, index))
        {
            lua_pushstring(L, "__cpp_data");
            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                return false;

            lua_pop(L, 1);

            lua_pushstring(L, "__cpp_type");
            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                return false;

            bool hasSameType = &typeid(boost::shared_ptr<T>*) == lua_touserdata(L, -1);

            lua_pop(L, 1);

            return hasSameType;
        }

        return false;
    }

    static reference_detector<boost::shared_ptr<T> > GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (lua_istable(L, index))
        {
            lua_pushstring(L, "__cpp_data");
            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            void * value = lua_touserdata(L, -1);
            lua_pop(L, 1);

            lua_pushstring(L, "__cpp_type");

            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            bool hasSameType = &typeid(boost::shared_ptr<T>*) == lua_touserdata(L, -1);

            lua_pop(L, 1);

            if (hasSameType)
                return reference_detector<boost::shared_ptr<T> >(*reinterpret_cast<boost::shared_ptr<T>*>(value), returnValueCount, L);

            throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


        } else if (lua_isnil(L, index)) {
            return reference_detector<boost::shared_ptr<T> >(boost::shared_ptr<T>(), returnValueCount, L);
        }

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");
    }

};


template <typename T>
class lua_choose_type<boost::shared_ptr<T>>
{
private:
    static std::map<void*, boost::shared_ptr<T> > instances;
    static int instanceTableReference;
public:
    static bool FindAndPushTable(lua_State *L, void * data)
    {
        if (instanceTableReference < 0)
        {
            lua_createtable(L, 8, 8);
            lua_createtable(L, 1, 1);
            lua_pushstring(L, "v");
            lua_setfield(L, -2, "__mode");
            lua_setmetatable(L, -2);

            instanceTableReference = luaL_ref(L, LUA_REGISTRYINDEX);



            return false;
        } else {
            std::stringstream stm;

            stm << data;

            lua_rawgeti(L, LUA_REGISTRYINDEX, instanceTableReference);

            lua_getfield(L, -1, stm.str().c_str());

            lua_remove(L, -2);

            if (!lua_isnil(L, -1))
            {
                return true;
            }

            lua_pop(L, 1);

            return false;

        }
    }

    static int DestroyObject(lua_State *L)
    {
        if (lua_istable(L, 1))
        {
            lua_pushstring(L, "__cpp_data");
            lua_gettable(L, 1);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            void * value = lua_touserdata(L, -1);
            lua_pop(L, 1);

            lua_pushstring(L, "__cpp_type");

            lua_gettable(L, 1);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            bool hasSameType = &typeid(boost::shared_ptr<T>*) == lua_touserdata(L, -1);

            lua_pop(L, 1);

            if (hasSameType)
            {
                instances.erase(reinterpret_cast<boost::shared_ptr<T>*>(value)->get());

                return 0;
            }

            throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


        }

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

    }

    static void Push(lua_State *L, const boost::shared_ptr<T>& data)
    {
        if (!FindAndPushTable(L, data.get()))
        {
            boost::shared_ptr<T> * sharedPtr;

            auto it = instances.find(data.get());

            if (it!=instances.end())
                sharedPtr = &it->second;
            else
                sharedPtr = &(instances.emplace(data.get(), data).first->second);


            lua_createtable(L, 2, 2);
            lua_pushlightuserdata(L, reinterpret_cast<void*>(sharedPtr));
            lua_setfield(L, -2, "__cpp_data");
            lua_pushlightuserdata(L,
                                  const_cast<void*>(reinterpret_cast<const void*>(&typeid(sharedPtr))));
            lua_setfield(L, -2, "__cpp_type");


            lua_rawgeti(L, LUA_REGISTRYINDEX, instanceTableReference);

            lua_pushvalue(L, -2);

            std::stringstream stm;

            stm << data.get();

            lua_setfield(L, -2, stm.str().c_str());

            lua_pop(L, 1);

            // meta-table
            lua_createtable(L, 2, 2);
            lua_pushcfunction(L, &DestroyObject);
            lua_setfield(L, -2, "__gc");
            lua_setmetatable(L, -2);



        }

    }

    static bool IsAtAddress(lua_State *L, int index)
    {
        if (lua_istable(L, index))
        {
            lua_pushstring(L, "__cpp_data");
            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                return false;

            lua_pop(L, 1);

            lua_pushstring(L, "__cpp_type");
            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                return false;

            bool hasSameType = &typeid(boost::shared_ptr<T>*) == lua_touserdata(L, -1);

            lua_pop(L, 1);

            return hasSameType;
        }

        return false;
    }

    static boost::shared_ptr<T>  GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (lua_istable(L, index))
        {
            lua_pushstring(L, "__cpp_data");
            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            void * value = lua_touserdata(L, -1);
            lua_pop(L, 1);

            lua_pushstring(L, "__cpp_type");

            lua_gettable(L, index);

            if (!lua_isuserdata(L, -1))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");

            bool hasSameType = &typeid(boost::shared_ptr<T>*) == lua_touserdata(L, -1);

            lua_pop(L, 1);

            if (hasSameType)
                return *reinterpret_cast<boost::shared_ptr<T>*>(value);

            throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


        } else if (lua_isnil(L, index)) {
            return reference_detector<boost::shared_ptr<T> >(boost::shared_ptr<T>(), returnValueCount, L);
        }

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");
    }

};


template <> class lua_choose_type<int32_t&>
{
public:
    static void Push(lua_State *L, int32_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static reference_detector<int32_t> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return reference_detector<int32_t>(lua_tonumber(L, index), returnValueCount, L);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<bool&>
{
public:
    static void Push(lua_State *L, bool data)
    {
        lua_pushboolean(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isboolean(L, index);
    };

    static reference_detector<bool> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return reference_detector<bool>(lua_toboolean(L, index), returnValueCount, L);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};



template <> class lua_choose_type<uint32_t&>
{
public:
    static void Push(lua_State *L, uint32_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static reference_detector<uint32_t> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return reference_detector<uint32_t>(lua_tonumber(L, index), returnValueCount, L);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<int16_t&>
{
public:
    static void Push(lua_State *L, int16_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static reference_detector<int16_t> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return reference_detector<int16_t>(lua_tonumber(L, index), returnValueCount, L);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<uint16_t&>
{
public:
    static void Push(lua_State *L, uint16_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static reference_detector<uint16_t> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return reference_detector<uint16_t>(lua_tonumber(L, index), returnValueCount, L);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<int8_t&>
{
public:
    static void Push(lua_State *L, int8_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static reference_detector<int8_t> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return reference_detector<int8_t>(lua_tonumber(L, index), returnValueCount, L);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<uint8_t&>
{
public:
    static void Push(lua_State *L, uint8_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static reference_detector<uint8_t> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return reference_detector<uint8_t>(lua_tonumber(L, index), returnValueCount, L);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<char&>
{
public:
    static void Push(lua_State *L, char data)
    {
        lua_pushlstring(L, &data, 1);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        bool isString = lua_isstring(L, index);

        if (!isString)
        {
            if (!lua_isinteger(L, index))
                return false;

            return true;
        }

        const char * p = lua_tostring(L, index);

        return *p==0 || p[1]==0;
    };

    static reference_detector<char> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        bool isString = lua_isstring(L, index);

        if (!isString)
        {
            if (!lua_isinteger(L, index))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


            return	reference_detector<char>(lua_tointeger(L, index), returnValueCount, L);
        }

        const char * p = lua_tostring(L, index);

        return reference_detector<char>(*p, returnValueCount, L);
    }
};

template<> class lua_choose_type<float&>
{
public:
    static void Push(lua_State *L, float data)
    {
        lua_pushnumber(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isnumber(L, index);
    };

    static reference_detector<float> GetValue(lua_State *L, int index, int &returnValueCount)
    {
        if (IsAtAddress(L, index))
            return reference_detector<float>(lua_tonumber(L, index), returnValueCount, L);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }

};

template<> class lua_choose_type<double&>
{
public:
    static void Push(lua_State *L, double data)
    {
        lua_pushnumber(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isnumber(L, index);
    };

    static reference_detector<double> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return reference_detector<double>(lua_tonumber(L, index), returnValueCount, L);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }

};

template<> class lua_choose_type<std::string>
{
public:
    static void Push(lua_State *L, std::string value)
    {
        lua_pushlstring(L, value.c_str(), value.length());
    }

    static bool IsAtAddress(lua_State * L, int index)
    {
        return lua_isstring(L, index);
    }


    static std::string GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
        {
            size_t len;
            auto s = lua_tolstring(L, index, &len);

            return std::string(s, len);
        }

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");
    }


};


template<> class lua_choose_type<const std::string&>
{
public:
    static void Push(lua_State *L, std::string value)
    {
        lua_pushlstring(L, value.c_str(), value.length());
    }

    static bool IsAtAddress(lua_State * L, int index)
    {
        return lua_isstring(L, index);
    }


    static std::string GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
        {
            size_t len = 0;

            const char * s = lua_tolstring(L, index, &len);

            return std::string(s, len);
        }

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");
    }


};


template<> class lua_choose_type<std::string&>
{
public:
    static void Push(lua_State *L, std::string value)
    {
        lua_pushlstring(L, value.c_str(), value.length());
    }

    static bool IsAtAddress(lua_State * L, int index)
    {
        return lua_isstring(L, index);
    }


    static reference_detector<std::string> GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
        {
            size_t len = 0;

            const char * s = lua_tolstring(L, index, &len);

            return reference_detector<std::string>(std::string(s, len), returnValueCount, L);
        }

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");
    }


};


// -- lua

template <> class lua_choose_type<bool>
{
public:
    static void Push(lua_State *L, bool data)
    {
        lua_pushboolean(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isboolean(L, index);
    };

    static bool GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return lua_toboolean(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<int32_t>
{
public:
    static void Push(lua_State *L, int32_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static int32_t GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return lua_tonumber(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<uint32_t>
{
public:
    static void Push(lua_State *L, uint32_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static uint32_t GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return lua_tonumber(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<int16_t>
{
public:
    static void Push(lua_State *L, int16_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static int16_t GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return lua_tonumber(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<uint16_t>
{
public:
    static void Push(lua_State *L, uint16_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static uint16_t GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return lua_tonumber(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<int8_t>
{
public:
    static void Push(lua_State *L, int8_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static int8_t GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return lua_tonumber(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<uint8_t>
{
public:
    static void Push(lua_State *L, uint8_t data)
    {
        lua_pushinteger(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isinteger(L, index);
    };

    static uint8_t GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return lua_tonumber(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }
};


template <> class lua_choose_type<char>
{
public:
    static void Push(lua_State *L, char data)
    {
        lua_pushlstring(L, &data, 1);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        bool isString = lua_isstring(L, index);

        if (!isString)
        {
            if (!lua_isinteger(L, index))
                return false;

            return true;
        }

        const char * p = lua_tostring(L, index);

        return *p==0 || p[1]==0;
    };

    static char GetValue(lua_State *L, int index, int& returnValueCount)
    {
        bool isString = lua_isstring(L, index);

        if (!isString)
        {
            if (!lua_isinteger(L, index))
                throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


            return	lua_tointeger(L, index);
        }

        const char * p = lua_tostring(L, index);

        return *p;
    }
};

template<> class lua_choose_type<float>
{
public:
    static void Push(lua_State *L, float data)
    {
        lua_pushnumber(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isnumber(L, index);
    };

    static float GetValue(lua_State *L, int index, int &returnValueCount)
    {
        if (IsAtAddress(L, index))
            return lua_tonumber(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }

};


template<> class lua_choose_type<double>
{
public:
    static void Push(lua_State *L, double data)
    {
        lua_pushnumber(L, data);
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isnumber(L, index);
    };

    static double GetValue(lua_State *L, int index, int& returnValueCount)
    {
        if (IsAtAddress(L, index))
            return lua_tonumber(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }

};


template <typename ret_t, typename...arg_t> class WrapFunctionForLua
{
public:
    typedef ret_t (*function_type)(arg_t...);


private:
    function_type function;
    MethodRegistryFunction registeringFunction;

public:
    WrapFunctionForLua(function_type function) : function(function),registeringFunction(nullptr)
    {
    }


    WrapFunctionForLua(function_type function, MethodRegistryFunction registeringFunction) : function(function),registeringFunction(registeringFunction)
    {
    }

    void Push(lua_State *L)
    {
        lua_pushlightuserdata(L, reinterpret_cast<void*>(function));
        lua_pushlightuserdata(L, reinterpret_cast<void*>(registeringFunction));
        lua_pushcclosure(L, &__lua_callroot, 2);
    }

private:
    static int __lua_callroot(lua_State *L)
    {
	int oldTop = lua_gettop(L);
	bool errorThrown = false;

	try {
	        int returnValueCount = 1;

        	function_type function = reinterpret_cast<function_type>(lua_touserdata(L, lua_upvalueindex(1)));

	        MethodRegistryFunction registeringFunction = reinterpret_cast<MethodRegistryFunction>(lua_touserdata(L, lua_upvalueindex(2)));


        	__call(function, returnValueCount, L);




        	// return value
	        if (registeringFunction!=nullptr)
        	    registeringFunction(L);


        	return returnValueCount;
	} catch (const std::exception& e)
	{
		lua_settop(L, oldTop);

		std::stringstream stm;

		stm << "Bridged C++ Exception: " << e.what();

		lua_pushstring(L, stm.str().c_str());
	
		errorThrown = true;
	} catch (...)
	{
		lua_settop(L, oldTop);

		lua_pushstring(L, "Bridged C++ Exception: Unknown exception");

		errorThrown = true;

	}

	if (errorThrown)
		lua_error(L);
    } 


    static void __call(function_type function, int& returnValueCount, lua_State *L)
    {
        int index = sizeof...(arg_t);

        lua_choose_type<ret_t>::Push(L, function(lua_choose_type<arg_t>::GetValue(L, index--, returnValueCount)...));
    }

};


template <typename...arg_t> class WrapFunctionForLua<void, arg_t...>
{
public:
    typedef void (*function_type)(arg_t...);

private:
    function_type function;

public:
    WrapFunctionForLua(function_type function) : function(function)
    {
    }

    void Push(lua_State *L)
    {
        lua_pushlightuserdata(L, reinterpret_cast<void*>(function));
        lua_pushcclosure(L, &__lua_callroot, 1);
    }

private:
    static int __lua_callroot(lua_State *L)
    {
	int oldTop = lua_gettop(L);
	bool errorThrown = false;

	try {
	        int returnValueCount = 0;

        	function_type function = reinterpret_cast<function_type>(lua_touserdata(L, lua_upvalueindex(1)));

        	__call(function, returnValueCount, L);


        	return returnValueCount;
	} catch (const std::exception& e)
	{
		lua_settop(L, oldTop);

		std::stringstream stm;

		stm << "Bridged C++ Exception: " << e.what();

		lua_pushstring(L, stm.str().c_str());

		errorThrown = true;
	} catch (...)
	{
		lua_settop(L, oldTop);

		lua_pushstring(L, "Bridged C++ Exception: Unknown exception");

		errorThrown = true;

	}

	if (errorThrown);
		lua_error(L);
    }


    static void __call(function_type function, int& returnValueCount, lua_State *L)
    {
        int index = sizeof...(arg_t);

        function(lua_choose_type<arg_t>::GetValue(L, index--, returnValueCount)...);
    }

};

template <typename funcType>
class PointerToMemberFunction
{
private:
    funcType pointer;


public:
    PointerToMemberFunction(funcType pointer) : pointer (pointer)
    {
    }

    funcType Get()
    {
        return pointer;
    }
};

template <typename CLASS, typename ret_t, typename...arg_t> class WrapMethodForLua
{
public:
    typedef ret_t (CLASS::* function_type)(arg_t...);

private:
    function_type function;

public:
    WrapMethodForLua(function_type function) : function(function)
    {
    }

    void Push(lua_State *L, boost::shared_ptr<CLASS>  object)
    {
        boost::shared_ptr<PointerToMemberFunction<function_type> > fn(new PointerToMemberFunction<function_type>(function));
        lua_choose_type<decltype(fn)>::Push(L, fn);
        lua_choose_type<decltype(object)>::Push(L, object);
        lua_pushcclosure(L, &__lua_callroot, 2);
    }

private:
    typedef boost::shared_ptr<PointerToMemberFunction<function_type> > shptr;

    static int __lua_callroot(lua_State *L)
    {
	int oldTop = lua_gettop(L);
	bool errorThrown = false;

	try {
	        int returnValueCount = 1;


        	int garbage;

	        shptr function = lua_choose_type<shptr>::GetValue(L,  lua_upvalueindex(1), garbage);
        	boost::shared_ptr<CLASS>* object = lua_choose_type<boost::shared_ptr<CLASS>*>::GetValue(L,  lua_upvalueindex(2), garbage);;

	        __call(*object, function, returnValueCount, L);


        	return returnValueCount;
	} catch (const std::exception& e)
	{
		lua_settop(L, oldTop);

		std::stringstream stm;

		stm << "Bridged C++ Exception: " << e.what();

		lua_pushstring(L, stm.str().c_str());

		errorThrown = true;
	} catch (...)
	{
		lua_settop(L, oldTop);

		lua_pushstring(L, "Bridged C++ Exception: Unknown exception");

		errorThrown = true;

	}


	if (errorThrown)
		lua_error(L);
    }


    static void __call(const boost::shared_ptr<CLASS>& object, const shptr& function, int& returnValueCount, lua_State *L)
    {
        int index = sizeof...(arg_t);

        lua_choose_type<ret_t>::Push(L, (object.get()->*(function.get()->Get()))(lua_choose_type<arg_t>::GetValue(L, index--, returnValueCount)...));
    }

};


template <typename CLASS, typename...arg_t> class WrapMethodForLua <CLASS, void, arg_t...>
{
public:
    typedef void (CLASS::* function_type)(arg_t...);

private:
    function_type function;

public:
    WrapMethodForLua(function_type function) : function(function)
    {
    }

    void Push(lua_State *L, boost::shared_ptr<CLASS>  object)
    {
        boost::shared_ptr<PointerToMemberFunction<function_type> > fn(new PointerToMemberFunction<function_type>(function));
        lua_choose_type<decltype(fn)>::Push(L, fn);
        lua_choose_type<decltype(object)>::Push(L, object);
        lua_pushcclosure(L, &__lua_callroot, 2);
    }

private:
    typedef boost::shared_ptr<PointerToMemberFunction<function_type> > shptr;

    static int __lua_callroot(lua_State *L)
    {
	int oldTop = lua_gettop(L);
	bool errorThrown = false;

	try {
	        int returnValueCount = 0;


        	int garbage;

	        shptr function = lua_choose_type<shptr>::GetValue(L,  lua_upvalueindex(1), garbage);
        	boost::shared_ptr<CLASS>* object = lua_choose_type<boost::shared_ptr<CLASS>*>::GetValue(L,  lua_upvalueindex(2), garbage);;

	        __call(*object, function, returnValueCount, L);


        	return returnValueCount;
	} catch (const std::exception& e)
	{
		lua_settop(L, oldTop);

		std::stringstream stm;

		stm << "Bridged C++ Exception: " << e.what();

		lua_pushstring(L, stm.str().c_str());

		errorThrown = true;
	} catch (...)
	{
		lua_settop(L, oldTop);

		lua_pushstring(L, "Bridged C++ Exception: Unknown exception");

		errorThrown = true;

	}

	if (errorThrown)
		lua_error(L);
    }


    static void __call(const boost::shared_ptr<CLASS>& object, const shptr& function, int& returnValueCount, lua_State *L)
    {
        int index = sizeof...(arg_t);

        (object.get()->*(function.get()->Get()))(lua_choose_type<arg_t>::GetValue(L, index--, returnValueCount)...);
    }

};

template <typename T>
inline reference_detector<T>::~reference_detector()
{
    if ((luaState!=nullptr) && wasReferenced)
        lua_choose_type<T>::Push(luaState, value);
}

class lua_returned_value
{
private:
    lua_State * luaState;
    int	    stackTopOld, stackTopNew;
    int 	    index;

public:

    lua_returned_value(lua_State * luaState, int stackTopOld, int stackTopNew, int index):
        luaState(luaState), stackTopOld(stackTopOld), stackTopNew(stackTopNew), index(index)
    {
    }


    template<typename T> operator T()
    {
        int count = 0;

        if (stackTopNew - stackTopOld >= index + 1)
            return lua_choose_type<T>::GetValue(luaState, -(index + 1), count);

        throw std::logic_error("No values was returned at that position");
    }

};

class lua_returned_values
{
private:
    lua_State * luaState;
    int	    stackTopOld, stackTopNew;
public:
    lua_returned_values(lua_State * luaState, int stackTopOld, int stackTopNew):
        luaState(luaState), stackTopOld(stackTopOld), stackTopNew(stackTopNew)
    {
    }

    lua_returned_values(lua_returned_values&& other)
    {
        luaState = other.luaState;
        luaState = nullptr;
        stackTopOld = other.stackTopOld;
        stackTopOld = 0;
        stackTopNew = other.stackTopNew;
        stackTopNew = 0;
    }

    ~lua_returned_values()
    {
        if (luaState!=nullptr)
            lua_pop(luaState, stackTopNew - stackTopOld);
    }

    template <typename T>
    operator T()
    {
        int count = 0;
        if (stackTopNew - stackTopOld >=1)
            return lua_choose_type<T>::GetValue(luaState, -1, count);

        throw std::logic_error("No values were returned");
    }


    lua_returned_value operator[](int index)
    {
        if (stackTopNew - stackTopOld >= -(index+1))
            return lua_returned_value(luaState, stackTopOld, stackTopNew, index);

        throw std::logic_error("No value exists at this index");
    }

    int GetReturnValueCount() const
    {
        return stackTopNew - stackTopOld;
    }





};

class LuaFunctionWrapper
{
private:
    lua_State * luaState;
    int	    stackLocation;
    int	    positionalIndex;
    int	    functionRef;

public:
    int GetStackLocation() const
    {
        return stackLocation;
    }

    int GetPositionalIndex() const
    {
        return positionalIndex;
    }

public:
    LuaFunctionWrapper(lua_State * luaState,  int positionalIndex): luaState(luaState), stackLocation(0), positionalIndex(positionalIndex)
    {
        lua_pushvalue(luaState, positionalIndex);
        functionRef = luaL_ref(luaState, LUA_REGISTRYINDEX);
    }

    LuaFunctionWrapper(const LuaFunctionWrapper& other): luaState(other.luaState), stackLocation(other.stackLocation), positionalIndex(other.positionalIndex)
    {
        lua_rawgeti(luaState, LUA_REGISTRYINDEX, other.functionRef);
        functionRef = luaL_ref(luaState, LUA_REGISTRYINDEX);
    }

    ~LuaFunctionWrapper()
    {
        luaL_unref(luaState, LUA_REGISTRYINDEX, functionRef);
    }

    template <typename...arg_t>
    lua_returned_values operator()(arg_t... args)
    {
        stackLocation = lua_gettop(luaState);

        lua_rawgeti(luaState, LUA_REGISTRYINDEX, functionRef);

        return _call<sizeof...(args)>(args...);
    }

private:
    template <int size, typename T, typename...arg_t>
    lua_returned_values _call(T firstArg, arg_t...args)
    {
        lua_choose_type<T>::Push(luaState, firstArg);

        return _call<size>(args...);
    }

    template <int size>
    lua_returned_values _call()
    {


        int callStatus = lua_pcall(luaState, size, LUA_MULTRET, 0);


        if (callStatus==0)
            return lua_returned_values(luaState, stackLocation, lua_gettop(luaState));

        std::string error = lua_tostring(luaState, -1);

        lua_pop(luaState, 1);

        throw std::logic_error(error);


    }

};


template<> class lua_choose_type<LuaFunctionWrapper>
{
public:
    static void Push(lua_State *L, LuaFunctionWrapper data)
    {
        lua_pushvalue(L, lua_gettop(L)-data.GetStackLocation() + data.GetPositionalIndex());
    };

    static bool IsAtAddress(lua_State *L, int index)
    {
        return lua_isfunction(L, index);
    };

    static LuaFunctionWrapper GetValue(lua_State *L, int index, int &returnValueCount)
    {
        if (IsAtAddress(L, index))
            return LuaFunctionWrapper(L, index);

        throw std::logic_error("Typemismatch: Lua pushed argument is of wrong type.");


    }

};

template <typename ret_t, typename...arg_t> void RegisterLuaFunction(lua_State * L, const char * functionName, ret_t (*cfunction)(arg_t...))
{
    WrapFunctionForLua<ret_t, arg_t...>(cfunction).Push(L);

    lua_setglobal(L, functionName);
}


template <typename CLASS, typename ret_t, typename...arg_t> void RegisterLuaMethod(lua_State * L, const char * functionName, const boost::shared_ptr<CLASS>& object, ret_t (CLASS::*cfunction)(arg_t...))
{
    WrapMethodForLua<CLASS, ret_t, arg_t...>(cfunction).Push(L, object);

    lua_setfield(L, -2, functionName);
}

template<typename CLASS> class StaticReference
{
public:
    static boost::shared_ptr<CLASS> CurrentInstance;
};

template<typename CLASS> boost::shared_ptr<CLASS> StaticReference<CLASS>::CurrentInstance;


template <typename CLASS, typename...arg_t> int RegisterLuaClass(lua_State * L, const char * functionName, MethodRegistryFunction registryFunction)
{
    struct __internal {
        static boost::shared_ptr<CLASS> create(arg_t...args)
        {
            StaticReference<CLASS>::CurrentInstance.reset(new CLASS(args...));

            return StaticReference<CLASS>::CurrentInstance;
        }
    };
    WrapFunctionForLua<boost::shared_ptr<CLASS>, arg_t...>(__internal::create, registryFunction).Push(L);

    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);

    lua_setglobal(L, functionName);

    return functionRef;
}


#define LUA_CLASS_METHODS [](lua_State* L) {

#define LUA_REGISTER_METHOD(CLASS, METHOD) RegisterLuaMethod(L, #METHOD, StaticReference<CLASS>::CurrentInstance, &CLASS::METHOD);

#define LUA_END_CLASS_METHODS(CLASS) StaticReference<CLASS>::CurrentInstance.reset(); }

template <typename T>
std::map<void*, boost::shared_ptr<T> > lua_choose_type<boost::shared_ptr<T> >::instances;

template <typename T> int lua_choose_type<boost::shared_ptr<T> >::instanceTableReference = -1;
}
