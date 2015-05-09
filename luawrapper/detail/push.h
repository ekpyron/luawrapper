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

template<typename T>
T *push (detail::if_not_pointer_t<T, lua_State> *L, T &&t) {
    T **obj = static_cast<T**> (lua_newuserdata (L, sizeof (T)));
    *obj = new T (std::move(t));
    lua_pushlightuserdata (L, *obj);
    detail::CreateMetatable<T> (L);
    lua_setmetatable (L, -3);
    lua_pop (L, 1);
    return *obj;
}

template<typename T>
T *push (detail::if_not_pointer_t<T, lua_State> *L, const T &t) {
    T **obj = static_cast<T**> (lua_newuserdata (L, sizeof (T)));
    *obj = new T (t);
    lua_pushlightuserdata (L, *obj);
    detail::CreateMetatable<T> (L);
    lua_setmetatable (L, -3);
    lua_pop (L, 1);
    return *obj;
}

template<typename T>
T push (detail::if_pointer_t<T, lua_State> *L, const T &t) {
    T *obj = static_cast<T*> (lua_newuserdata (L, sizeof (T)));
    *obj = t;
    lua_pushlightuserdata (L, (void*)t);
    detail::CreateMetatable<typename std::remove_pointer<T>::type> (L);
    lua_setmetatable (L, -3);
    lua_pop (L, 1);
    return *obj;
}

} /* namespace lua */
