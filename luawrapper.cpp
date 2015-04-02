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

    lua_pushlightuserdata (L, &registrydummy);
    lua_newtable (L);
    lua_settable (L, LUA_REGISTRYINDEX);
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

int State::registrydummy = 0;

} /* namespace lua */
