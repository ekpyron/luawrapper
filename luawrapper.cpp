#include "luawrapper.h"

namespace lua {
namespace detail {

void AddToTables (lua_State *L, const function *ptr, const size_t &size)
{
    for (auto i = 0; i < size; i++) {
        switch (ptr[i].type) {
            case function::MEMBERFUNCTION:
                // push upvalue
                lua_pushvalue (L, -3);
                // push closure
                lua_pushcclosure (L, ptr[i].func, 1);
                // add to index table
                lua_setfield (L, -2, ptr[i].name);
                break;
            case function::STATICFUNCTION:
                // push function
                lua_pushcfunction (L, ptr[i].func);
                // add to index table
                lua_setfield (L, -2, ptr[i].name);
                break;
            case function::METAFUNCTION:
                // push upvalue
                lua_pushvalue (L, -3);
                // push closure
                lua_pushcclosure (L, ptr[i].func, 1);
                // add to index table
                lua_setfield (L, -3, ptr[i].name);
                break;
            case function::BASECLASS:
                AddToTables (L, ptr[i].ptr, ptr[i].size);
                break;
            case function::INDEXFUNCTION:
                // create new metatable
                lua_newtable (L);
                // push upvalue
                lua_pushvalue (L, -4);
                // push closure
                lua_pushcclosure (L, ptr[i].func, 1);
                // add to new metatable
                lua_setfield (L, -2, "__index");
                // set metatable
                lua_setmetatable (L, -2);
                break;
        }
    }
}

void AddToStaticTables (lua_State *L, const function *ptr, const size_t &size)
{
    for (auto i = 0; i < size; i++) {
        switch (ptr[i].type) {
            case function::STATICFUNCTION:
                // register function
                lua_pushcfunction (L, ptr[i].func);
                lua_setfield (L, -3, ptr[i].name);
                break;
            case function::CONSTRUCTOR:
                // register constructors
                lua_pushcfunction (L, ptr[i].func);
                lua_setfield (L, -2, "__call");
                break;
        }
    }
}

void CreateMetatable (lua_State *L, const functionlist &functions)
{
    // create metatable
    lua_newtable (L);

    // create index table
    lua_newtable (L);

    // populate index table
    AddToTables (L, functions.begin (), functions.size ());

    // register index table
    lua_setfield (L, -2, "__index");
}

void RegisterClass (lua_State *L, const char *name, const functionlist &functions)
{
    // create table
    lua_newtable (L);

    // create metatable
    lua_newtable (L);

    AddToStaticTables (L, functions.begin (), functions.size ());

    // set metatable
    lua_setmetatable (L, -2);

    // set global
    lua_setfield (L, LUA_GLOBALSINDEX, name);
}

} /* namespace detail */


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

State::~State (void)
{
    lua_close (L);
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

Reference::Reference (const Reference &r) : L (r.L)
{
    if (r.valid ()) {
        lua_rawgeti (L, LUA_REGISTRYINDEX, r.ref);
        ref = luaL_ref (L, LUA_REGISTRYINDEX);
    } else {
        L = nullptr; ref = LUA_NOREF;
    }
}
Reference &Reference::operator= (const Reference &r)
{
    reset ();
    if (r.valid ()) {
        L = r.L;
        lua_rawgeti (L, LUA_REGISTRYINDEX, r.ref);
        ref = luaL_ref (L, LUA_REGISTRYINDEX);
    }
    return *this;
}
Reference::~Reference (void)
{
    reset ();
}
Reference &Reference::operator= (Reference &&r)
{
    reset ();
    L= r.L; r.L = nullptr;
    ref = r.ref; r.ref = LUA_NOREF;
    return *this;
}
void Reference::reset (void)
{
    if (L != nullptr && ref != LUA_NOREF) {
        luaL_unref (L, LUA_REGISTRYINDEX, ref);
        L = nullptr; ref = LUA_NOREF;
    }
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
WeakReference::~WeakReference (void)
{
    reset ();
}
WeakReference &WeakReference::operator= (WeakReference &&r)
{
    reset ();
    L= r.L; r.L = nullptr;
    ref = r.ref; r.ref = LUA_NOREF;
    return *this;
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

} /* namespace lua */