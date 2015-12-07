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
#include "luawrapper.h"

namespace lua {


WeakReference::WeakReference (lua_State *L_, const int &index) : L (L_)
{
    State::push_weak_registry (L);
    lua_pushvalue (L, index < 0 ? index - 1 : index);
    ref = luaL_ref (L, -2);
    lua_pop (L, 1);
}


WeakReference::WeakReference (const WeakReference &r) : L (r.L)
{
    if (r.valid ()) {
        State::push_weak_registry (L);
        lua_rawgeti (L, -1, r.ref);
        ref = luaL_ref (L, -2);
        lua_pop (L, 1);
    } else {
        L = nullptr; ref = LUA_NOREF;
    }
}
WeakReference::WeakReference (const Reference &r) : L (r.L) {
    if (r.valid ()) {
        State::push_weak_registry (L);
        lua_rawgeti (L, LUA_REGISTRYINDEX, r.ref);
        ref = luaL_ref (L, -2);
        lua_pop (L, 1);
    } else {
        L = nullptr; ref = LUA_NOREF;
    }
}
WeakReference::~WeakReference (void)
{
    reset ();
}
WeakReference &WeakReference::operator= (const WeakReference &r)
{
    reset ();
    if (r.valid ()) {
        L = r.L;
        State::push_weak_registry (L);
        lua_rawgeti (L, -1, r.ref);
        ref = luaL_ref (L, -2);
        lua_pop (L, 1);
    }
    return *this;
}
WeakReference &WeakReference::operator= (const Reference &r) {
    if (r.valid ()) {
        L = r.L;
        State::push_weak_registry (L);
        lua_rawgeti (L, LUA_REGISTRYINDEX, r.ref);
        ref = luaL_ref (L, -2);
        lua_pop (L, 1);
    } else {
        L = nullptr; ref = LUA_NOREF;
    }
    return *this;
}
WeakReference &WeakReference::operator= (WeakReference &&r) noexcept
{
    reset ();
    L= r.L; r.L = nullptr;
    ref = r.ref; r.ref = LUA_NOREF;
    return *this;
}
bool WeakReference::valid (void) const
{
    if (L == nullptr || ref == LUA_NOREF || ref == LUA_REFNIL) return false;
    State::push_weak_registry (L);
    lua_rawgeti (L, -1, ref);
    bool v = !lua_isnil (L, -1);
    lua_pop (L, 2);
    return v;
}
void WeakReference::reset (void)
{
    if (L != nullptr && ref != LUA_NOREF) {
        State::push_weak_registry (L);
        luaL_unref (L, -1, ref);
        lua_pop (L, 1);
        L = nullptr; ref = LUA_NOREF;
    }
}

void WeakReference::push (void) const
{
    if (L != nullptr) {
        if (ref != LUA_NOREF && ref != LUA_REFNIL) {
            State::push_weak_registry (L);
            lua_rawgeti (L, -1, ref);
            lua_remove (L, -2);
        } else {
            lua_pushnil (L);
        }
    }
}

WeakReference Type<WeakReference>::pull (lua_State *L, const int &index)
{
    return WeakReference (L, index);
}

void Type<WeakReference>::push (lua_State *L, const WeakReference &v)
{
    State::push_weak_registry (L);
    lua_rawgeti (L, -1, v.ref);
    lua_remove (L, -2);
}

} /* namespace lua */
