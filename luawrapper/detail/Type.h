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

template<typename T, class = void>
struct Type;

template<typename T, class>
struct Type
{
    static T &pull (lua_State *L, const int &index) {
        return **static_cast<T**> (lua_touserdata (L, index));
    }
    static bool check (lua_State *L, const int &index) {
        return lua_isnil (L, index)
               || lua_isuserdata (L, index)
                  && detail::CheckType (L, index, typeid (T).hash_code ());
    }
    static void push (lua_State *L, T &&v) {
        lua::push (L, std::move (v));
    }
    static void push (lua_State *L, const T &v) {
        lua::push (L, v);
    }
};

template<typename T>
struct Type<T, typename std::enable_if<std::is_pointer<T>::value>::type>
{
    static T &pull (lua_State *L, const int &index) {
        return *static_cast<T*> (lua_touserdata (L, index));
    }
    static bool check (lua_State *L, const int &index) {
        return lua_isnil (L, index)
               || lua_isuserdata (L, index)
                  && detail::CheckType (L, index, typeid (typename std::remove_pointer<T>::type).hash_code ());
    }
    static void push (lua_State *L, T &&v) {
        lua::push (L, std::move (v));
    }
    static void push (lua_State *L, const T &v) {
        lua::push (L, v);
    }
};

template<typename T>
struct Type<T, typename std::enable_if<std::numeric_limits<T>::is_integer>::type>
{
    static T pull (lua_State *L, const int &index) {
        return lua_tointeger (L, index);
    }
    static bool check (lua_State *L, const int &index) {
        return lua_isnumber (L, index);
    }
    static void push (lua_State *L, T v) {
        lua_pushinteger (L, v);
    }
};

template<typename T>
struct Type<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
{
    static void push (lua_State *L, T v) { lua_pushnumber (L, v); }
    static T pull (lua_State *L, const int &index) { return lua_tonumber (L, index); }
    static bool check (lua_State *L, const int &index) { return lua_isnumber (L, index); }
};

template<>
struct Type<lua_State*> {
    static lua_State *pull (lua_State *L, const int &index) { return L; }
    static bool check (lua_State *L, const int &index) { return true; }
};

template<>
struct Type<bool> {
    static bool check (lua_State *L, const int &index) { return lua_isboolean (L, index); }
    static bool pull (lua_State *L, const int &index) { return lua_toboolean (L, index); }
    static void push (lua_State *L, bool v) { lua_pushboolean (L, v); }
};

template<>
struct Type<std::string>
{
    static bool check (lua_State *L, const int &index) { return lua_isstring (L, index); }
    static std::string pull (lua_State *L, const int &index) {
        size_t len = 0;
        const char *str = lua_tolstring (L, index, &len);
        return std::string (str, len);
    }
    static void push (lua_State *L, const std::string &v) { lua_pushlstring (L, v.data (), v.length ()); }
};

template<>
struct Type<ManualReturn>
{
    static void push (lua_State *L, const ManualReturn &v) { }
};

template<>
struct Type<Reference>
{
    static bool check (lua_State *L, const int &index) { return true; }
    static Reference pull (lua_State *L, const int &index) {
        return Reference (L, index);
    }
    static void push (lua_State *L, const Reference &t) {
        lua_rawgeti (L, LUA_REGISTRYINDEX, t.ref);
    }
};

template<typename T>
struct Type<TypedReference<T>>
{
    static bool check (lua_State *L, const int &index) { return true; }
    static TypedReference<T> pull (lua_State *L, const int &index) {
        return TypedReference<T> (L, index);
    }
    static void push (lua_State *L, const TypedReference<T> &t) {
        lua_rawgeti (L, LUA_REGISTRYINDEX, t.ref);
    }
};

template<>
struct Type<WeakReference>
{
    static bool check (lua_State *L, const int &index) { return true; }
    static WeakReference pull (lua_State *L, const int &index);
    static void push (lua_State *L, const WeakReference &v);
};

} /* namespace lua */
