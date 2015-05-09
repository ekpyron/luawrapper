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
namespace lua {

template<typename T, typename... Args>
struct Constructor
{
    static bool Wrap (lua_State *L, int &results) {
        T **ptr = static_cast<T**> (lua_newuserdata (L, sizeof (T*)));
        try {
            *ptr = lua::detail::ConstructHelper<T, Args...>::construct (L, 2);
        } catch (...) {
            lua_pop (L, 1);
            throw;
        }
        if (*ptr == nullptr) {
            lua_pop (L, 1);
            results = 0;
            return false;
        }
        lua_pushlightuserdata (L, *ptr);
        lua::detail::CreateMetatable<T> (L);
        lua_setmetatable (L, -3);
        lua_pop (L, 1);
        results = 1;
        return true;
    }
    static int Wrap (lua_State *L) {
        int result;
        Wrap (L, result);
        return result;
    }
};
template<typename T, typename... Args>
struct ConstructorWithSelfReference {
    static bool Wrap (lua_State *L, int &results) {
        T **ptr = static_cast<T**> (lua_newuserdata(L, sizeof(T*)));
        lua_pushvalue (L, -1);
        lua_insert (L, 2);
        try {
            *ptr = lua::detail::ConstructHelper<T, Reference, Args...>::construct(L, 2);
        } catch (...) {
            lua_remove (L, 2);
            lua_pop (L, 1);
            throw;
        }
        if (*ptr == nullptr) {
            lua_pop (L, 1);
            results = 0;
            return false;
        }
        lua_pushlightuserdata (L, *ptr);
        lua::detail::CreateMetatable<T> (L);
        lua_setmetatable(L, -3);
        lua_pop (L, 1);
        results = 1;
        return true;
    }
    static int Wrap (lua_State *L) {
        int results;
        Wrap (L, results);
        return results;
    }
};

template<typename T>
struct Destructor {
    static int Wrap (lua_State *L) noexcept {
        T *obj = static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1)));
        delete obj;
        return 0;
    }
};

} /* namespace lua */
