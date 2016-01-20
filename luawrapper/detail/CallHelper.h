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
namespace detail {

template<typename Retval, typename T, typename... Args>
struct CallHelper
{
private:
    template<int S>
    using argtype = typename tuple_element<S, Args...>::type;
    template<int S>
    using arghandler = ArgHandler<S, argtype<S>>;
    using rettype = Type<typename detail::baretype<Retval>::type>;
    template<typename R, typename FN, int ...S>
    static int do_call (if_not_void_t<R, lua_State> *L, int startindex, T *t, FN fn, seq<S...>) {
        rettype::push (L, (t->*fn) (arghandler<S>::get(L, startindex)...));
        return 1;
    }
    template<typename R, typename FN, int ...S>
    static int do_call (if_void_t<R, lua_State> *L, int startindex, T *t, FN fn, seq<S...>) {
        (t->*fn) (arghandler<S>::get (L, startindex)...);
        return 0;
    }
    template<int ...S>
    static bool checkargs (lua_State *L, int startindex, seq<S...>) {
        if (lua_gettop (L) != startindex + sizeof... (Args) - 1) return false;
        return alltrue (arghandler<S>::check (L, startindex)...);
    }
public:
    template<typename FN>
    static bool try_call (lua_State *L, int &results, int startindex, T *t, FN fn) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return false;
        try {
            results = do_call<Retval> (L, startindex, t, fn, typename gens<sizeof...(Args)>::type ());
        } catch (const std::exception &e) {
            luaL_error (L, "Lua error: %s", e.what ());
        } catch (...) {
            luaL_error (L, "Lua error: unknown exception.");
        }
        return true;
    }
    template<typename FN>
    static int call (lua_State *L, int startindex, T *t, FN fn) {
        int results;
        if (!try_call (L, results, startindex, t, fn))
            luaL_error (L, "Invalid arguments.");
        return results;
    }
};

template<typename Retval, typename... Args>
struct StaticCallHelper
{
private:
    template<int S>
    using argtype = typename tuple_element<S, Args...>::type;
    template<int S>
    using arghandler = ArgHandler<S, argtype<S>>;
    using rettype = Type<typename detail::baretype<Retval>::type>;
    template<typename R, typename FN, int ...S>
    static int do_call (if_not_void_t<R, lua_State> *L, int startindex, FN fn, seq<S...>) {
        rettype::push (L, (*fn) (arghandler<S>::get(L, startindex)...));
        return 1;
    }
    template<typename R, typename FN, int ...S>
    static int do_call (if_void_t<R, lua_State> *L, int startindex, FN fn, seq<S...>) {
        (*fn) (arghandler<S>::get (L, startindex)...);
        return 0;
    }
    template<int ...S>
    static bool checkargs (lua_State *L, int startindex, seq<S...>) {
        if (lua_gettop (L) != startindex + sizeof... (Args) - 1) return false;
        return alltrue (arghandler<S>::check (L, startindex)...);
    }
public:
    template<typename FN>
    static bool try_call (lua_State *L, int &results, int startindex, FN fn) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return false;
        try {
            results = do_call<Retval> (L, startindex, fn, typename gens<sizeof...(Args)>::type ());
        } catch (const std::exception &e) {
            luaL_error (L, "Lua error: %s", e.what ());
        } catch (...) {
            luaL_error (L, "Lua error: unknown exception.");
        }
        return true;
    }
    template<typename FN>
    static int call (lua_State *L, int startindex, FN fn) {
        int results;
        if (!try_call (L, results, startindex, fn))
            luaL_error (L, "Invalid arguments.");
        return results;
    }
};

} /* namespace detail */
} /* namespace lua */
