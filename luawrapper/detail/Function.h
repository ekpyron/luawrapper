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

template<typename Retval, typename... Args>
struct Function;

template<typename Retval, typename... Args>
struct Function<Retval(Args...)> {
    template<typename T, Retval (T::*M) (Args...),
            int skipargs = -detail::count_tuple_elements<lua_State*, Args...>::value>
    static int Wrap (lua_State *L) {
        return lua::detail::CallHelper<Retval, T, Args...>::call
                (L, 1 + skipargs, static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1))), M);
    }
    template<Retval (*M) (Args...), int skipargs = -detail::count_tuple_elements<lua_State*, Args...>::value>
    static int Wrap (lua_State *L) {
        return lua::detail::StaticCallHelper<Retval, Args...>::call
                (L, 1 + skipargs, M);
    }
    template<typename T, Retval (T::*M) (Args...),
            int skipargs = -detail::count_tuple_elements<lua_State*, Args...>::value>
    static bool Wrap (lua_State *L, int &results) {
        return lua::detail::CallHelper<Retval, T, Args...>::try_call
                (L, results, 1 + skipargs, static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1))), M);
    }
    template<Retval (*M) (Args...), int skipargs = -detail::count_tuple_elements<lua_State*, Args...>::value>
    static bool Wrap (lua_State *L, int &results) {
        return lua::detail::StaticCallHelper<Retval, Args...>::try_call
                (L, results, 1 + skipargs, M);
    }
};

template<typename Retval, typename... Args>
struct Function<Retval(Args...)const> {
    template<typename T, Retval (T::*M) (Args...) const,
            int skipargs = -detail::count_tuple_elements<lua_State*, Args...>::value>
    static bool Wrap (lua_State *L, int &results) {
        return lua::detail::CallHelper<Retval, T, Args...>::try_call
                (L, results, 1 + skipargs, static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1))), M);
    }
    template<typename T, Retval (T::*M) (Args...) const,
            int skipargs = -detail::count_tuple_elements<lua_State*, Args...>::value>
    static int Wrap (lua_State *L) {
        return lua::detail::CallHelper<Retval, T, Args...>::call
                (L, 1 + skipargs, static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1))), M);
    }
};

} /* namespace lua */
