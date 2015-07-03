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

State::State (void) : L (luaL_newstate ())
{
    if (L == nullptr) throw std::runtime_error ("Cannot create a lua state.");

    // create empty table
    lua_newtable (L);

    // setup metatable for weak references
    lua_newtable (L);
    lua_pushliteral (L, "v");
    lua_setfield (L, -2, "__mode");
    lua_setmetatable (L, -2);

    // store in registry
    int ref = luaL_ref (L, LUA_REGISTRYINDEX);
    if (ref != 1) {
        throw std::runtime_error ("Unexpected registry key for weak reference table.");
    }
}

State::State (State &&state) : L (state.L)
{
    state.L = nullptr;
}

State::~State (void)
{
    if (L) lua_close (L);
}

State &State::operator= (State &&state) noexcept
{
    L = state.L; state.L = nullptr;
    return *this;
}

void State::loadlib (const lua_CFunction &fn, const std::string &name)
{
    lua_pushcfunction (L, fn);
    lua_pushlstring (L, name.data (), name.size ());
    lua_call (L, 1, 0);
}

void State::push_weak_registry (lua_State *L)
{
    lua_rawgeti (L, LUA_REGISTRYINDEX, 1);
}

} /* namespace lua */
