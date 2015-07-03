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
class TypedReference : public Reference {
public:
    typedef typename std::conditional<std::is_pointer<T>::value, T, T*>::type ptrtype;
    TypedReference (lua_State *L, const int &index) : Reference (L, index) {
        if (!checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
    }
    TypedReference (void) : Reference () {
    }
    TypedReference (TypedReference<T> &&r) : Reference (r) {
    }
    TypedReference (const TypedReference<T> &r) : Reference (r) {
    }
    TypedReference (Reference &&r) : Reference (r) {
        if (r.ref != LUA_REFNIL && !checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
    }
    TypedReference (const Reference &r) : Reference (r) {
        if (!checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
    }
    operator T (void) const {
        return convert<T> ();
    }
    ptrtype operator-> (void) {
        return static_cast<ptrtype> (ptr);
    }
    const ptrtype operator-> (void) const {
        return static_cast<const ptrtype> (ptr);
    }
    TypedReference<T> &operator= (const Reference &r) {
        if (!r.checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
        Reference::operator= (r);
        return *this;
    }
    TypedReference<T> &operator= (Reference &&r) {
        if (!r.checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
        Reference::operator= (r);
        return *this;
    }
    TypedReference<T> &operator= (const TypedReference<T> &r) {
        Reference::operator= (r);
        return *this;
    }
    TypedReference<T> &operator= (TypedReference<T> &&r) noexcept {
        Reference::operator= (r);
        return *this;
    }
};

} /* namespace lua */
