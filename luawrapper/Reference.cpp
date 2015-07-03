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

Reference::Reference (lua_State *_L, const int &index) : L (_L), ptr (nullptr)
{
    lua_pushvalue (L, index);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (lua_isuserdata (L, index)) {
        ptr = reinterpret_cast<void**> (lua_touserdata (L, index));
    }
}

Reference::Reference (const Reference &r) : L (r.L), ptr (r.ptr)
{
    if (r.valid ()) {
        lua_rawgeti (L, LUA_REGISTRYINDEX, r.ref);
        ref = luaL_ref (L, LUA_REGISTRYINDEX);
    } else {
        L = nullptr; ref = LUA_NOREF; ptr = nullptr;
    }
}

Reference::Reference (const WeakReference &r) : L (r.L), ptr (nullptr), ref (LUA_NOREF)
{
    if (L) {
        r.push ();
        if (lua_isuserdata (L, -1)) {
            ptr = reinterpret_cast<void**> (lua_touserdata (L, -1));
        }
        ref = luaL_ref(L, LUA_REGISTRYINDEX);
    }
}

Reference &Reference::operator= (const Reference &r)
{
    reset ();
    if (r.valid ()) {
        L = r.L;
        lua_rawgeti (L, LUA_REGISTRYINDEX, r.ref);
        ref = luaL_ref (L, LUA_REGISTRYINDEX);
        ptr = r.ptr;
    }
    return *this;
}
Reference::~Reference (void)
{
    reset ();
}
Reference &Reference::operator= (Reference &&r) noexcept
{
    reset ();
    L= r.L; r.L = nullptr;
    ref = r.ref; r.ref = LUA_NOREF;
    ptr = r.ptr; r.ptr = nullptr;
    return *this;
}
bool Reference::operator< (const Reference &r) const
{
    if (ptr < r.ptr) return true;
    if (r.ptr < ptr) return false;
    if (ptr != nullptr) return false;
    if (L < r.L) return true;
    if (r.L < L) return false;
    push ();
    r.push ();
    bool result = lua_lessthan (L, -1, -2);
    lua_pop (L, 2);
    return result;
}
bool Reference::operator== (const Reference &r) const
{
    if (ptr != r.ptr) return false;
    if (ptr != nullptr) return true;
    if (L != r.L) return false;
    push ();
    r.push ();
    bool result = lua_equal (L, -1, -2);
    lua_pop (L, 2);
    return result;
}
void Reference::reset (void)
{
    if (L != nullptr && ref != LUA_NOREF) {
        luaL_unref (L, LUA_REGISTRYINDEX, ref);
    }
    L = nullptr;
    ref = LUA_NOREF;
    ptr = nullptr;
}

void Reference::push (void) const {
    if (L != nullptr) {
        if (ref != LUA_NOREF && ref != LUA_REFNIL) {
            lua_rawgeti (L, LUA_REGISTRYINDEX, ref);
        } else {
            lua_pushnil (L);
        }
    }
}

bool Reference::isnil (void) const {
    bool result = true;
    if (L != nullptr) {
        if (ref != LUA_NOREF && ref != LUA_REFNIL) {
            lua_rawgeti (L, LUA_REGISTRYINDEX, ref);
            result = lua_isnil (L, -1);
            lua_pop (L, 1);
        }
    }
    return result;
}

} /* namespace lua */
