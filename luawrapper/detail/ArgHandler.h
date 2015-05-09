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

template<typename T>
struct pass_as_rvalue
        : std::integral_constant<bool,
                std::is_rvalue_reference<T>::value
                && std::is_reference<decltype(Type<typename detail::baretype<T>::type>::pull
                        (std::declval<lua_State*> (), std::declval<const int&> ()))>::value> { };

template<int N, typename T, bool = pass_as_rvalue<T>::value>
struct ArgHandler;

template<int N, typename T>
struct ArgHandler<N, T, false>
{
    using type = Type<typename detail::baretype<T>::type>;
    static bool check (lua_State *L, int startindex) {
        return type::check (L, startindex + N);
    }
    static decltype (type::pull (std::declval<lua_State*> (), std::declval<const int&> ()))
    get (lua_State *L, int startindex) {
        return type::pull (L, startindex + N);
    }
};

template<int N, typename T>
struct ArgHandler<N, T, true>
{
    using type = Type<typename detail::baretype<T>::type>;
    static bool check (lua_State *L, int startindex) {
        return type::check (L, startindex + N);
    }
    static decltype (std::move (type::pull (std::declval<lua_State*> (), std::declval<const int&> ())))
    get (lua_State *L,int startindex) {
        return std::move (type::pull (L, startindex + N));
    }
};

} /* namespace detail */
} /* namespace lua */
