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

// template helpers
template<typename T>
using baretype = std::remove_cv<typename std::remove_reference<T>::type>;
template<int ...> struct seq { };
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> { };
template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };
template <int N, typename... U> struct tuple_element;
template <typename U0, typename... U> struct tuple_element<0, U0, U...> { typedef U0 type; };
template <int N, typename U0, typename... U> struct tuple_element<N, U0, U...> {
    typedef typename tuple_element<N-1, U...>::type type;
};
inline bool alltrue (void) { return true; }
template<typename... Args> bool alltrue (bool u, Args... args) { return u && alltrue (args...); }

template<typename... Args>
struct count_tuple_elements;

template<typename T>
struct count_tuple_elements<T> : std::integral_constant<int, 0> {};

template<typename T, typename T0, typename... Args>
struct count_tuple_elements<T, T0, Args...>
        : std::integral_constant<int, count_tuple_elements<T, Args...>::value
                                      + (std::is_same<T, T0>::value ? 1 : 0)> {};

template<typename R, typename T>
using if_void_t = typename std::enable_if<std::is_same<R, void>::value, T>::type;
template<typename R, typename T>
using if_not_void_t = typename std::enable_if<!std::is_same<R, void>::value, T>::type;

template<typename R, typename T = void>
using if_pointer_t = typename std::enable_if<std::is_pointer<R>::value, T>::type;
template<typename R, typename T = void>
using if_not_pointer_t = typename std::enable_if<!std::is_pointer<R>::value, T>::type;

} /* namespace detail */
} /* namespace lua */
