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

template<typename T, typename... Args>
struct ConstructHelper
{
private:
    template<int S>
    using argtype = typename tuple_element<S, Args...>::type;
    template<int ...S>
    static T *construct (lua_State *L, int startindex, seq<S...>) {
        try {
            return new T (ArgHandler<S, argtype<S>>::get (L, startindex)...);
        } catch (std::exception &e) {
            luaL_error (L, "Lua error: %s", e.what ());
        } catch (...) {
            luaL_error (L, "Lua error: unknown exception.");
        }
    }
    template<int ...S>
    static bool checkargs (lua_State *L, int startindex, seq<S...>) {
        if (lua_gettop (L) != startindex + sizeof... (Args)) return false;
        return alltrue (ArgHandler<S, argtype<S>>::check (L, startindex)...);
    }
public:
    static T *construct (lua_State *L, int startindex) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return nullptr;
        return construct (L, startindex, typename gens<sizeof...(Args)>::type ());
    }
};

} /* namespace detail */
} /* namespace lua */
