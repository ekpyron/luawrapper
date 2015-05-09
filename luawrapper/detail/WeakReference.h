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

class WeakReference {
public:
    WeakReference (void) : L (nullptr), ref (LUA_NOREF) {}
    WeakReference (lua_State *L, const int &index);
    WeakReference (WeakReference &&r) : L (r.L), ref (r.ref) {
        r.L = nullptr; r.ref = LUA_NOREF;
    }
    WeakReference (const WeakReference &r);
    WeakReference (const Reference &r);
    WeakReference &operator= (const WeakReference &r);
    WeakReference &operator= (WeakReference &&r) noexcept;
    WeakReference &operator= (const Reference &r);
    ~WeakReference (void);
    bool valid (void) const;
    operator const int &(void) const { return ref; }
    template<typename T>
    decltype(Type<T, void>::pull (nullptr, 0)) convert (void) const;
    template<typename T>
    bool checktype (void) const;
    void reset (void);
    lua_State* const &GetLuaState (void) const { return L; }
    void push (void) const;
private:
    lua_State *L;
    int ref;
    friend class Reference;
    template<typename, class>
    friend struct Type;
};

template<typename T>
inline decltype(Type<T, void>::pull (nullptr, 0)) WeakReference::convert (void) const {
    struct StackHelper {
        StackHelper (const WeakReference *ref) : L (ref->GetLuaState ()){
            State::push_weak_registry (L);
            lua_rawgeti (L, -1, ref->ref);
        }
        ~StackHelper (void) {
            lua_pop (L, 2);
        }
        lua_State *L;
    } stackhelper (this);
    return Type<T, void>::pull (L, -1);
}

template<typename T>
bool WeakReference::checktype (void) const {
    if (L == nullptr || ref == LUA_NOREF || ref == LUA_REFNIL) return false;
    State::push_weak_registry (L);
    lua_rawgeti (L, -1, ref);
    bool result = Type<T, void>::check (L, -1);
    lua_pop (L, 2);
    return result;
}


} /* namespace lua */
