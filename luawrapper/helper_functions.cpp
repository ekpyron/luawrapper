/*
 * C++ helper and wrapper functions for Lua.
 *
 * Copyright (c) 2015 Daniel Kirchner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "luawrapper.h"

namespace lua {
namespace detail {

void AddToTables (lua_State *L, const function *ptr, const size_t &size, std::vector<size_t> &typehashs, bool destructor) noexcept
{
    for (auto i = 0; i < size; i++) {
        switch (ptr[i].type) {
            case function::MEMBERFUNCTION:
                // push upvalue
                lua_pushvalue (L, -3);
                // push closure
                lua_pushcclosure (L, ptr[i].func, 1);
                // add to index table
                lua_setfield (L, -2, ptr[i].name);
                break;
            case function::STATICFUNCTION:
                // push function
                lua_pushcfunction (L, ptr[i].func);
                // add to index table
                lua_setfield (L, -2, ptr[i].name);
                break;
            case function::METAFUNCTION:
                // push upvalue
                lua_pushvalue (L, -3);
                // push closure
                lua_pushcclosure (L, ptr[i].func, 1);
                // add to meta table
                lua_setfield (L, -3, ptr[i].name);
                break;
            case function::DESTRUCTOR:
                if (destructor) {
                    // push upvalue
                    lua_pushvalue (L, -3);
                    // push closure
                    lua_pushcclosure (L, ptr[i].func, 1);
                    // add to meta table
                    lua_setfield (L, -3, "__gc");
                }
                break;
            case function::BASECLASS:
                typehashs.push_back (ptr[i].hashcode);
                AddToTables (L, ptr[i].listptr->begin (), ptr[i].listptr->size (), typehashs, false);
                break;
            case function::INDEXFUNCTION:
                // create new metatable
                lua_newtable (L);
                // push upvalue
                lua_pushvalue (L, -4);
                // push closure
                lua_pushcclosure (L, ptr[i].func, 1);
                // add to new metatable
                lua_setfield (L, -2, "__index");
                // set metatable of index table
                lua_setmetatable (L, -2);
                break;
        }
    }
}

bool CheckType (lua_State *L, const int &index, const size_t &typehash)
{
    static_assert (sizeof (lua_Number) == sizeof (size_t), "lua_Number and size_t have different sizes");
    lua_getmetatable (L, index);
    if (lua_isnil (L, -1)) return false;
    lua_getfield (L, -1, "__ctypes");
    if (lua_isnil (L, -1)) return false;
    int i = 1;
    while (true) {
        lua_rawgeti (L, -1, i++);
        if (lua_isnil (L, -1)) {
            lua_pop (L, 3);
            return false;
        }
        lua_Number v = lua_tonumber (L, -1);
        if (*reinterpret_cast<size_t*> (&v) == typehash) {
            lua_pop (L, 3);
            return true;
        }
        lua_pop (L, 1);
    }
}

void AddToStaticTables (lua_State *L, const function *ptr, const size_t &size) noexcept
{
    for (auto i = 0; i < size; i++) {
        switch (ptr[i].type) {
            case function::STATICFUNCTION:
                // register function
                lua_pushcfunction (L, ptr[i].func);
                lua_setfield (L, -3, ptr[i].name);
                break;
            case function::CONSTRUCTOR:
                // register constructors
                lua_pushcfunction (L, ptr[i].func);
                lua_setfield (L, -2, "__call");
                break;
        }
    }
}

void CreateMetatable (lua_State *L, const functionlist &functions, const size_t &typehash, bool destructor) noexcept
{
    static_assert (sizeof (lua_Number) == sizeof (size_t), "lua_Number and size_t have different sizes");

    std::vector<size_t> typehashs;
    typehashs.push_back (typehash);
    // create metatable
    lua_newtable (L);

    // create index table
    lua_newtable (L);

    // populate index table
    AddToTables (L, functions.begin (), functions.size (), typehashs, destructor);

    // register index table
    lua_setfield (L, -2, "__index");

    // create type table
    lua_newtable (L);
    for (auto i = 0; i < typehashs.size (); i++) {
        lua_pushnumber (L, *reinterpret_cast<lua_Number*> (&typehashs[i]));
        lua_rawseti (L, -2, i + 1);
    }
    lua_setfield (L, -2, "__ctypes");
}
} /* namespace detail */

void register_class (lua_State *L, const char *name, const functionlist &functions)
{
    // create table
    lua_newtable (L);

    // create metatable
    lua_newtable (L);

    detail::AddToStaticTables (L, functions.begin (), functions.size ());

    // set metatable
    lua_setmetatable (L, -2);

    // set global
    lua_setfield (L, LUA_GLOBALSINDEX, name);
}

} /* namespace lua */
