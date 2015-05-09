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

class function;
typedef std::initializer_list<function> functionlist;
template<typename T>
struct Functions;

namespace detail {
void AddToTables (lua_State *L, const function *ptr, const size_t &size, std::vector<size_t> &typehashs, bool destructor = true) noexcept;
void AddToStaticTables (lua_State *L, const function *ptr, const size_t &size) noexcept;
bool CheckType (lua_State *L, const int &index, const size_t &typehash);
void CreateMetatable (lua_State *L, const functionlist &functions, const size_t &typehash, bool destructor = true) noexcept;
template<typename T>
void CreateMetatable (lua_State *L, bool destructor = true) noexcept {
    CreateMetatable (L, Functions<T>::value, typeid (T).hash_code (), destructor);
}
void register_class (lua_State *L, const char *name, const functionlist &functions);

} /* namespace detail */
} /* namespace lua */
