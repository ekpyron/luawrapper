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

class WeakReference;

template<typename T, class>
struct Type;

class Reference {
public:
    Reference (lua_State *L, const int &index);
    Reference (void) : L (nullptr), ref (LUA_NOREF), ptr (nullptr) {}
    Reference (Reference &&r) : L (r.L), ref (r.ref), ptr (r.ptr) {
        r.L = nullptr; r.ref = LUA_NOREF; r.ptr = nullptr;
    }
    Reference (const Reference &r);
    explicit Reference (const WeakReference &r);
    Reference &operator= (const Reference &r);
    Reference &operator= (Reference &&r) noexcept;
    ~Reference (void);
    bool valid (void) const {
        return L != nullptr && ref != LUA_NOREF && ref != LUA_REFNIL;
    }
    bool isnil (void) const;
    template<typename T, detail::if_not_pointer_t<T>* = nullptr>
    decltype(Type<T, void>::pull (nullptr, 0)) convert (void) const;
    template<typename T, detail::if_pointer_t<T>* = nullptr>
    T convert (void) const;
    template<typename T, detail::if_not_pointer_t<T>* = nullptr>
    bool checktype (void) const;
    template<typename T, detail::if_pointer_t<T>* = nullptr>
    bool checktype (void) const;
    void reset (void);
    lua_State* const &GetLuaState (void) const { return L; }
    void push (void) const;
    bool operator< (const Reference &r) const;
    bool operator== (const Reference &r) const;
    bool operator!= (const Reference &r) const {
        return !operator== (r);
    }
private:
    lua_State *L;
    int ref;
    void **ptr;
    friend class WeakReference;
    template<typename T>
    friend class TypedReference;
    template<typename, class>
    friend struct Type;
};

template<typename T, detail::if_not_pointer_t<T>*>
inline decltype(Type<T, void>::pull (nullptr, 0)) Reference::convert (void) const {
    struct StackHelper {
        StackHelper (const Reference *ref) : L (ref->GetLuaState ()) {
            ref->push ();
        }
        ~StackHelper (void) {
            lua_pop (L, 1);
        }
        lua_State *L;
    } stackhelper (this);
    return Type<T, void>::pull (L, -1);
}

template<typename T, detail::if_pointer_t<T>*>
T Reference::convert (void) const {
    return (ptr != nullptr) ? (*reinterpret_cast<T*const> (ptr)) : nullptr;
}

template<typename T, detail::if_not_pointer_t<T>*>
bool Reference::checktype (void) const {
    if (L == nullptr || ref == LUA_NOREF || ref == LUA_REFNIL) return false;
    lua_rawgeti (L, LUA_REGISTRYINDEX, ref);
    bool result = Type<T, void>::check (L, -1);
    lua_pop (L, 1);
    return result;
}

template<typename T, detail::if_pointer_t<T>*>
bool Reference::checktype (void) const {
    if (ref == LUA_REFNIL || ref == LUA_NOREF) return true;
    return checktype<typename std::remove_pointer<T>::type> ();
}

} /* namespace lua */
