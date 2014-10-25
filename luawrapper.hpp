/*
 * C++ helper and wrapper functions for Lua.
 *
 * Copyright (c) 2014 Daniel Kirchner
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
#ifndef LUAWRAPPER_HPP
#define LUAWRAPPER_HPP

#include <string>
#include <memory>
#include <sstream>
#include <vector>

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace luawrapper {

namespace detail {
inline void ArgsToStringStream (std::stringstream &stream) {
}

template<typename T, typename... Args>
inline void ArgsToStringStream (std::stringstream &stream, T t, Args... args) {
	stream << t;
	ArgsToStringStream (stream, args...);
}
} /* namespace detail */

template<typename... Args>
inline void LuaThrow (lua_State *L, Args... args)
{
    std::stringstream stream;
    stream << "LUA ERROR: ";
    detail::ArgsToStringStream (stream, args...);
    lua_pushstring (L, stream.str ().c_str ());
    lua_error (L);
}

namespace detail {
template<typename T>
struct classname;

template<typename T>
struct metatablename;

template<typename T> void GetFunctions (std::vector<luaL_Reg> &memfuncs, std::vector<luaL_Reg> &staticfuncs,
		std::vector<luaL_Reg> &metafuncs);

template<typename T>
struct is_shared_ptr {
     static constexpr bool value = false;
};

template<typename T>
struct is_shared_ptr<std::shared_ptr<T>> {
    static constexpr bool value = true;
};

template<typename T>
struct remove_ptr {
    typedef typename std::remove_pointer<T>::type type;
};

template<typename T>
struct remove_ptr<std::shared_ptr<T>> {
    typedef T type;
};

template<typename T>
bool CheckBase (lua_State *L, int index)
{
    lua_getfield (L, LUA_REGISTRYINDEX, detail::metatablename<typename detail::remove_ptr<T>::type>::value);
    // -1: metatable of T

    if (!lua_getmetatable (L, index < 0 ? index - 1 : index))
    {
        lua_pop (L, 1);
        return false;
    }
    // -1: metatable of index
    // -2: metatable of T

    while (true)
    {
        if (lua_rawequal (L, -1, -2))
        {
            lua_pop (L, 2);
            return true;
        }
        // -1: metatable of index
        // -2: metatable of T

        if (!lua_istable (L, -1))
        {
            lua_pop (L, 2);
            return false;
        }

        lua_getfield (L, -1, "__index");
        // -1: metatable of metatable of index
        // -2: metatable of index
        // -3: metatable of T

        if (lua_isnil (L, -1) || lua_rawequal (L, -1, -2))
        {
            lua_pop (L, 3);
            return false;
        }

        lua_remove (L, -2);
        // -1: metatable of metatable of index
        // -2: metatable of T
    }
}

template<typename T>
T *CastUserData (lua_State *L, int index)
{
    void *p = lua_touserdata (L, index);
    if (p == nullptr)
        return nullptr;

    if (!detail::CheckBase<T> (L, index))
        return nullptr;

    return reinterpret_cast<T*> (p);
}

inline bool AllTrue (void) {
    return true;
}

inline bool AllTrue (bool arg) {
    return arg;
}

template<typename... Args>
bool AllTrue (bool arg, Args... args) {
    return arg && AllTrue (args...);
}

template<typename T>
typename std::enable_if<std::is_constructible<T>::value && !std::is_arithmetic<T>::value
    && !std::is_same<T, std::string>::value, bool>::type
CheckLuaStackArg (lua_State *L, int index)
{
    if (lua_isnil (L, index))
        return true;
    if (!lua_isuserdata (L, index))
        return false;
    return CheckBase<T> (L, index);
}

template<typename T>
typename std::enable_if<!std::is_constructible<T>::value && !std::is_arithmetic<T>::value
    && !std::is_same<T, std::string>::value, bool>::type
CheckLuaStackArg (lua_State *L, int index)
{
    if (!lua_isuserdata (L, index))
        return false;
    return CheckBase<T> (L, index);
}

template<typename T>
typename std::enable_if<std::is_same<T, bool>::value, bool>::type CheckLuaStackArg (lua_State *L, int index)
{
    if (!lua_isboolean (L, index))
        return false;
    return true;
}

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, bool>::type
CheckLuaStackArg (lua_State *L, int index)
{
    if (!lua_isnumber (L, index))
        return false;
    return true;
}

template<typename T>
inline typename std::enable_if<std::is_same<T, std::string>::value, bool>::type
CheckLuaStackArg (lua_State *L, int index)
{
    if (!lua_isstring (L, index))
        return false;
    return true;
}

template<typename T>
typename std::enable_if<!std::is_constructible<T>::value && !std::is_arithmetic<T>::value
    && !std::is_same<typename std::remove_cv<T>::type, std::string>::value, T>::type
GetLuaStackArg (lua_State *L, int &index)
{
    index--;
    void *p = lua_touserdata (L, index);
    if (p == NULL)
        LuaThrow (L, "Cannot get user data from stack.");
    return *reinterpret_cast<T*> (p);
}

template<typename T>
typename std::enable_if<std::is_constructible<T>::value && !std::is_arithmetic<T>::value
    && !std::is_same<typename std::remove_cv<T>::type, std::string>::value, T>::type
GetLuaStackArg (lua_State *L, int &index)
{
    index--;
    if (lua_isnil (L, index))
        return T();
    void *p = lua_touserdata (L, index);
    if (p == NULL)
        LuaThrow (L, "Cannot get user data from stack.");
    return *reinterpret_cast<T*> (p);
}

template<typename T>
typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, bool>::value, bool>::type
GetLuaStackArg (lua_State *L, int &index)
{
    index--;
    return lua_toboolean (L, index);
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value && !std::is_same<typename std::remove_cv<T>::type, bool>::value,
T>::type GetLuaStackArg (lua_State *L, int &index)
{
    index--;
    return T (lua_tointeger (L, index));
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value
    && !std::is_same<typename std::remove_cv<T>::type, bool>::value, T>::type
GetLuaStackArg (lua_State *L, int &index)
{
    index--;
    return T (lua_tonumber (L, index));
}

template<typename T>
inline typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, std::string>::value, T>::type
GetLuaStackArg (lua_State *L, int &index)
{
    index--;
    size_t len;
    const char *str = lua_tolstring (L, index, &len);
    return std::string (str, len);
}

template<typename... Args>
struct CheckStackArgs
{
    CheckStackArgs (int startindex = 0) : index (startindex + sizeof... (Args)) {}

    bool check (lua_State *L) {
        if (index != lua_gettop (L) + 1)
            return false;
        return AllTrue (CheckArg<typename std::remove_cv<typename std::remove_reference<Args>::type>::type> (L)...);
    }
private:
    int index;
    template<typename T>
    bool CheckArg (lua_State *L)
    {
        index--;
        return CheckLuaStackArg<T> (L, index);
    }
};

template<typename Object, typename Returntype, typename... Args>
struct ExpandAndCall
{
    ExpandAndCall (int startindex = 0) : index (startindex + sizeof... (Args)) {}
    Returntype expand (lua_State *L, Object *object, Returntype (Object::*memberfunction) (Args...)) {
        return (object->*memberfunction)
                (GetLuaStackArg<typename std::remove_cv<typename std::remove_reference<Args>::type>::type>
                    (L, index)...);
    }
private:
    int index;
};

template<typename Object, typename Returntype, typename... Args>
struct ExpandAndCallConst
{
    ExpandAndCallConst (int startindex = 0) : index (startindex + sizeof... (Args)) {}
    Returntype expand (lua_State *L, Object *object, Returntype (Object::*memberfunction) (Args...) const) {
        return (object->*memberfunction)
                (GetLuaStackArg<typename std::remove_cv<typename std::remove_reference<Args>::type>::type>
                    (L, index)...);
    }
private:
    int index;
};

} /* namespace detail */

template<typename T>
int AutoDestructor (lua_State *L)
{
    if (lua_gettop (L) != 1)
        LuaThrow (L, "LUA ERROR: Invalid number of arguments");
    T *ptr = detail::CastUserData<T> (L, 1);
    if (ptr == nullptr)
        LuaThrow (L, "LUA ERROR: destructor called with invalid argument");
    ptr->~T ();
    return 0;
}

template<typename T, typename... Args>
T *NewUserData (lua_State *L, Args... args)
{
    T *ptr = reinterpret_cast<T*> (lua_newuserdata (L, sizeof (T)));
    new (ptr) T (args...);

    luaL_getmetatable (L, detail::metatablename<typename detail::remove_ptr<T>::type>::value);
    lua_setmetatable (L, -2);
    return ptr;
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
bool Pull (lua_State *L, int index, T *value)
{
    if (detail::CheckLuaStackArg<T> (L, index))
    {
        index++;
        *value = detail::GetLuaStackArg<T> (L, index);
        return true;
    }
    return false;
}

template<typename T>
T Pull (lua_State *L, int index)
{
    if (detail::CheckLuaStackArg<T> (L, index))
    {
        index++;
        return detail::GetLuaStackArg<T> (L, index);
    }
    LuaThrow (L, "LUA ERROR: invalid type at index ", index);
}

//////////////////////////////////////////////////////////////////////////


template<typename Object, typename Returntype, typename... Args>
typename std::enable_if<std::is_void<Returntype>::value, bool>::type
TryMemberCallFromLuaStack (lua_State *L, Object *object, int startindex,
        Returntype (Object::*memberfunction) (Args...))
{
    if (detail::CheckStackArgs<Args...> (startindex).check (L))
    {
        detail::ExpandAndCall<Object, Returntype, Args...> (startindex).expand (L, object, memberfunction);
        return true;
    }
    return false;
}

template<typename Object, typename Returntype, typename... Args>
typename std::enable_if<std::is_void<Returntype>::value, bool>::type
TryMemberCallFromLuaStack (lua_State *L, Object *object, int startindex,
        Returntype (Object::*memberfunction) (Args...) const)
{
    if (detail::CheckStackArgs<Args...> (startindex).check (L))
    {
        detail::ExpandAndCallConst<Object, Returntype, Args...> (startindex).expand (L, object, memberfunction);
        return true;
    }
    return false;
}


template<typename Object, typename Returntype, typename... Args>
typename std::enable_if<!std::is_void<Returntype>::value, bool>::type
TryMemberCallFromLuaStack (lua_State *L, Object *object, int startindex,
        typename std::add_pointer<typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type>::type
        returnvalue,
        Returntype (Object::*memberfunction) (Args...))
{
    if (detail::CheckStackArgs<Args...> (startindex).check (L))
    {
        *returnvalue = detail::ExpandAndCall<Object, Returntype, Args...> (startindex).expand (L, object, memberfunction);
        return true;
    }
    return false;
}

template<typename Object, typename Returntype, typename... Args>
typename std::enable_if<!std::is_void<Returntype>::value, bool>::type
TryMemberCallFromLuaStack (lua_State *L, Object *object, int startindex,
        typename std::add_pointer<typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type>::type
        returnvalue,
        Returntype (Object::*memberfunction) (Args...) const)
{
    if (detail::CheckStackArgs<Args...> (startindex).check (L))
    {
        *returnvalue = detail::ExpandAndCallConst<Object, Returntype, Args...> (startindex).expand (L, object, memberfunction);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, void>::type push2lua (lua_State *L, T t)
{
    lua_pushnumber (L, lua_Number (t));
}

template<typename T>
typename std::enable_if<!std::is_arithmetic<T>::value
    && std::is_same<typename std::remove_cv<typename std::remove_reference<T>::type>::type,
        std::string>::value, void>::type push2lua (lua_State *L, T t)
{
    lua_pushstring (L, t.c_str ());
}

template<typename T>
typename std::enable_if<!std::is_arithmetic<T>::value
    && !detail::is_shared_ptr<T>::value && !std::is_pointer<T>::value
    && !std::is_same<typename std::remove_cv<typename std::remove_reference<T>::type>::type,
        std::string>::value, void>::type push2lua (lua_State *L, T t)
{
    NewUserData<T> (L, t);
}

template<typename T>
typename std::enable_if<!std::is_arithmetic<T>::value
    && (detail::is_shared_ptr<T>::value || std::is_pointer<T>::value), void>::type push2lua (lua_State *L, T t)
{
    if (!t) lua_pushnil (L);
    else NewUserData<T> (L, t);
}

//////////////////////////////////////////////////////////////////////////

template<typename Returntype, typename Object, typename... Args>
typename std::enable_if<std::is_void<Returntype>::value, int>::type
AutoWrap (lua_State *L, Returntype (Object::*func) (Args...))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    Object *ptr = detail::CastUserData<Object> (L, 1);
    if (ptr == nullptr) LuaThrow (L, "LUA ERROR: Cannot convert argument to object of type: ",
    		detail::classname<typename detail::remove_ptr<Object>::type>::value);
    if (TryMemberCallFromLuaStack(L, ptr, 2, func))
        return 0;
    LuaThrow (L, "Invalid arguments");
}

template<typename Returntype, typename Object, typename... Args>
typename std::enable_if<!std::is_void<Returntype>::value, int>::type
AutoWrap (lua_State *L, Returntype (Object::*func) (Args...))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    Object *ptr = detail::CastUserData<Object> (L, 1);
    if (ptr == nullptr) LuaThrow (L, "LUA ERROR: Cannot convert argument to object of type: ",
    		detail::classname<typename detail::remove_ptr<Object>::type>::value);
    BaseReturntype retval;
    if (TryMemberCallFromLuaStack(L, ptr, 2, &retval, func))
    {
        push2lua (L, retval);
        return 1;
    }
    LuaThrow (L, "Invalid arguments");
}

template<typename Returntype, typename Object>
typename std::enable_if<std::is_void<Returntype>::value, int>::type
AutoWrap (lua_State *L, Returntype (Object::*func) (void))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    Object *ptr = detail::CastUserData<Object> (L, 1);
    if (ptr == nullptr) LuaThrow (L, "LUA ERROR: Cannot convert argument to object of type: ",
    		detail::classname<typename detail::remove_ptr<Object>::type>::value);

    (ptr->*func) ();
    return 0;
}

template<typename Returntype, typename Object>
typename std::enable_if<!std::is_void<Returntype>::value, int>::type
AutoWrap (lua_State *L, Returntype (Object::*func) (void))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    Object *ptr = detail::CastUserData<Object> (L, 1);
    if (ptr == nullptr) LuaThrow (L, "LUA ERROR: Cannot convert argument to object of type: ",
    		detail::classname<typename detail::remove_ptr<Object>::type>::value);

    BaseReturntype retval = (ptr->*func) ();
    push2lua (L, retval);
    return 1;
}

template<typename Returntype, typename Object, typename... Args>
int AutoWrap (lua_State *L, Returntype (Object::*func) (Args...) const)
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    Object *ptr = detail::CastUserData<Object> (L, 1);
    if (ptr == nullptr) LuaThrow (L, "LUA ERROR: Cannot convert argument to object of type: ",
    		detail::classname<typename detail::remove_ptr<Object>::type>::value);
    BaseReturntype retval;
    if (TryMemberCallFromLuaStack(L, ptr, 2, &retval, func))
    {
        push2lua (L, retval);
        return 1;
    }
    LuaThrow (L, "Invalid arguments");
}

template<typename Returntype, typename Object>
int AutoWrap (lua_State *L, Returntype (Object::*func) (void) const)
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    Object *ptr = detail::CastUserData<Object> (L, 1);
    if (ptr == nullptr) LuaThrow (L, "LUA ERROR: Cannot convert argument to object of type: ",
    		detail::classname<typename detail::remove_ptr<Object>::type>::value);
    BaseReturntype retval = (ptr->*func) ();
    push2lua (L, retval);
    return 1;
}

//////////////////////////////////////////////////////////////////////////

template<typename Returntype, typename Object, typename... Args>
typename std::enable_if<!std::is_void<Returntype>::value, int>::type
AutoWrapPtr (lua_State *L, Returntype (Object::*func) (Args...))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<Object*> (L, 1);
    BaseReturntype retval;
    if (TryMemberCallFromLuaStack(L, ptr, 2, &retval, func))
    {
        push2lua (L, retval);
        return 1;
    }
    LuaThrow (L, "Invalid arguments");
}

template<typename Returntype, typename Object, typename... Args>
typename std::enable_if<std::is_void<Returntype>::value, int>::type
AutoWrapPtr (lua_State *L, Returntype (Object::*func) (Args...))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<Object*> (L, 1);
    if (TryMemberCallFromLuaStack(L, ptr, 2, func))
        return 0;
    LuaThrow (L, "Invalid arguments");
}


template<typename Returntype, typename Object>
typename std::enable_if<!std::is_void<Returntype>::value, int>::type
AutoWrapPtr (lua_State *L, Returntype (Object::*func) (void))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<Object*> (L, 1);
    BaseReturntype retval = (ptr->*func) ();
    push2lua (L, retval);
    return 1;
}

template<typename Returntype, typename Object>
typename std::enable_if<std::is_void<Returntype>::value, int>::type
AutoWrapPtr (lua_State *L, Returntype (Object::*func) (void))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<Object*> (L, 1);
    (ptr->*func) ();
    return 0;
}

template<typename Returntype, typename Object, typename... Args>
int AutoWrapPtr (lua_State *L, Returntype (Object::*func) (Args...) const)
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<Object*> (L, 1);
    BaseReturntype retval;
    if (TryMemberCallFromLuaStack(L, ptr, 2, &retval, func))
    {
        push2lua (L, retval);
        return 1;
    }
    LuaThrow (L, "Invalid arguments");
}

template<typename Returntype, typename Object>
int AutoWrapPtr (lua_State *L, Returntype (Object::*func) (void) const)
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<Object*> (L, 1);
    BaseReturntype retval = (ptr->*func) ();
    push2lua (L, retval);
    return 1;
}

///////////////////////////////////////////////

template<typename Returntype, typename Object, typename... Args>
typename std::enable_if<std::is_void<Returntype>::value, int>::type
AutoWrapSharedPtr (lua_State *L, Returntype (Object::*func) (Args...))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<std::shared_ptr<Object>> (L, 1);
    if (TryMemberCallFromLuaStack(L, ptr.get(), 2, func))
        return 0;
    LuaThrow (L, "Invalid arguments");
}

template<typename Returntype, typename Object, typename... Args>
typename std::enable_if<!std::is_void<Returntype>::value, int>::type
AutoWrapSharedPtr (lua_State *L, Returntype (Object::*func) (Args...))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<std::shared_ptr<Object>> (L, 1);
    BaseReturntype retval;
    if (TryMemberCallFromLuaStack(L, ptr.get(), 2, &retval, func))
    {
        push2lua (L, retval);
        return 1;
    }
    LuaThrow (L, "Invalid arguments");
}

template<typename Returntype, typename Object>
typename std::enable_if<!std::is_void<Returntype>::value, int>::type
AutoWrapSharedPtr (lua_State *L, Returntype (Object::*func) (void))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<std::shared_ptr<Object>> (L, 1);
    BaseReturntype retval = ((ptr.get ())->*func) ();
    push2lua (L, retval);
    return 1;
}

template<typename Returntype, typename Object>
typename std::enable_if<std::is_void<Returntype>::value, int>::type
AutoWrapSharedPtr (lua_State *L, Returntype (Object::*func) (void))
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<std::shared_ptr<Object>> (L, 1);
    ((ptr.get ())->*func) ();
    return 0;
}

template<typename Returntype, typename Object, typename... Args>
int AutoWrapSharedPtr (lua_State *L, Returntype (Object::*func) (Args...) const)
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<std::shared_ptr<Object>> (L, 1);
    BaseReturntype retval;
    if (TryMemberCallFromLuaStack(L, ptr.get (), 2, &retval, func))
    {
        push2lua (L, retval);
        return 1;
    }
    LuaThrow (L, "Invalid arguments");
}

template<typename Returntype, typename Object>
int AutoWrapSharedPtr (lua_State *L, Returntype (Object::*func) (void) const)
{
    typedef typename std::remove_cv<typename std::remove_reference<Returntype>::type>::type BaseReturntype;
    auto ptr = Pull<std::shared_ptr<Object>> (L, 1);
    BaseReturntype retval = ((ptr.get())->*func) ();
    push2lua (L, retval);
    return 1;
}

///////////////////////////////////////////////

template<typename T>
void register_class (lua_State *L)
{
    luaL_newmetatable (L, detail::metatablename<T>::value);
    lua_pushstring (L, "__index");
    lua_pushvalue (L, -2);
    lua_settable (L, -3);
    std::vector<luaL_Reg> memfuncs, staticfuncs;
    detail::GetFunctions<T> (memfuncs, staticfuncs, memfuncs);
    memfuncs.push_back ({ NULL, NULL });
    luaL_register (L, NULL, memfuncs.data ());

    if (!staticfuncs.empty ())
    {
        staticfuncs.push_back ({ NULL, NULL });
        luaL_register (L, detail::classname<T>::value, staticfuncs.data ());
    }

    lua_pop (L, 1);
}

template<typename B, typename T>
void register_class (lua_State *L)
{
    std::vector<luaL_Reg> memfuncs, staticfuncs, metafuncs;
    detail::GetFunctions<T> (memfuncs, staticfuncs, metafuncs);

    lua_newtable (L);
    // -1: newtable

    lua_pushvalue (L, -1);
    // -1: newtable
    // -2: newtable

    lua_setmetatable (L, -2);
    // -1: newtable

    luaL_newmetatable (L, detail::metatablename<T>::value);
    // -1: metatable
    // -2: newtable

    if (!metafuncs.empty ())
    {
        metafuncs.push_back ({ NULL, NULL });
        luaL_register (L, NULL, metafuncs.data ());
    }

    lua_pushstring (L, "__index");
    // -1: __index
    // -2: metatable
    // -3: newtable

    lua_pushvalue (L, -3);
    // -1: newtable
    // -2: __index
    // -3: metatable
    // -4: newtable

    lua_settable (L, -3); // metatable[__index] = newtable
    // -1: metatable
    // -2: newtable

    lua_pop (L, 1);
    // -1: newtable

    lua_pushstring (L, "__index");
    // -1: __index
    // -2: newtable

    luaL_getmetatable (L, detail::metatablename<B>::value);
    // -1: supermetatable
    // -2: __index
    // -3: newtable


    lua_settable (L, -3); // newtable[__index] = supermetatable
    // -1: newtable

    memfuncs.push_back ({ NULL, NULL });
    luaL_register (L, NULL, memfuncs.data ());

    if (!staticfuncs.empty ())
    {
        staticfuncs.push_back ({ NULL, NULL });
        luaL_register (L, detail::classname<T>::value, staticfuncs.data ());
    }

    lua_pop (L, 1);
}


} /* namespace luawrapper */

#define LUAWRAPPER_DECLARE_CLASS(NAME) namespace luawrapper { namespace detail {\
	template<> struct classname<NAME> { static constexpr const char *value = #NAME; };\
	template<> struct metatablename<NAME> { static constexpr const char *value = "LUAWRAPPER." #NAME; };\
	}}

#define LUAWRAPPER_DECLARE_CLASS_RENAME(NAME,LUANAME) namespace luawrapper { namespace detail {\
	template<> struct classname<NAME> { static constexpr const char *value = LUANAME; };\
	template<> struct metatablename<NAME> { static constexpr const char *value = "LUAWRAPPER." LUANAME; };\
	}}

#define LUAWRAPPER_IMPORT_CLASS(NAME) namespace luawrapper { namespace detail {\
    extern template void GetFunctions<NAME> (std::vector<luaL_Reg> &memfuncs, std::vector<luaL_Reg> &staticfuncs,\
    		std::vector<luaL_Reg> &metafuncs);\
	}}

#define LUAWRAPPER_BEGIN_CLASS_WRAPPER(NAME) namespace luawrapper { namespace detail {\
	template<> void GetFunctions<NAME> (std::vector<luaL_Reg> &_memfuncs, std::vector<luaL_Reg> &_staticfuncs,\
    		std::vector<luaL_Reg> &_metafuncs) {\
	typedef NAME classtype;

#define LUAWRAPPER_BEGIN_STATIC_WRAPPER(NAME) _staticfuncs.push_back ({ NAME,\
	[] (lua_State *L) -> int { try {
#define LUAWRAPPER_END_STATIC_WRAPPER() } catch (std::exception &e) {\
	luawrapper::LuaThrow (L, e.what ()); }}});

#define LUAWRAPPER_BEGIN_MEMBER_WRAPPER(NAME) _memfuncs.push_back ({ NAME,\
	[] (lua_State *L) -> int { try {
#define LUAWRAPPER_END_MEMBER_WRAPPER() } catch (std::exception &e) {\
	luawrapper::LuaThrow (L, e.what ()); }}});

#define LUAWRAPPER_BEGIN_METAFUNCTION_WRAPPER(NAME) _metafuncs.push_back ({ NAME,\
	[] (lua_State *L) -> int { try {
#define LUAWRAPPER_END_METAFUNCTION_WRAPPER() } catch (std::exception &e) {\
	luawrapper::LuaThrow (L, e.what ()); }}});

#define LUAWRAPPER_MEMBER_WRAPPER(NAME) LUAWRAPPER_BEGIN_MEMBER_WRAPPER(#NAME)\
	return luawrapper::AutoWrap (L,&classtype::NAME);\
	LUAWRAPPER_END_MEMBER_WRAPPER()

#define LUAWRAPPER_MEMBER_WRAPPER_PTR(NAME) LUAWRAPPER_BEGIN_MEMBER_WRAPPER(#NAME)\
	return luawrapper::AutoWrapPtr (L,&classtype::NAME);\
	LUAWRAPPER_END_MEMBER_WRAPPER()

#define LUAWRAPPER_MEMBER_WRAPPER_SHARED_PTR(NAME) LUAWRAPPER_BEGIN_MEMBER_WRAPPER(#NAME)\
	return luawrapper::AutoWrapSharedPtr (L,&classtype::NAME);\
	LUAWRAPPER_END_MEMBER_WRAPPER()
	
#define LUAWRAPPER_MEMBER_WRAPPER_EX(RETVAL,NAME,...) LUAWRAPPER_BEGIN_MEMBER_WRAPPER(#NAME)\
        return AutoWrap<RETVAL, classtype, ##__VA_ARGS__> (L, &classtype::NAME);\
        LUAWRAPPER_END_MEMBER_WRAPPER()

#define LUAWRAPPER_MEMBER_WRAPPER_PTR_EX(RETVAL,NAME,...) LUAWRAPPER_BEGIN_MEMBER_WRAPPER(#NAME)\
        return AutoWrapPtr<RETVAL, classtype, ##__VA_ARGS__> (L, &classtype::NAME);\
        LUAWRAPPER_END_MEMBER_WRAPPER()

#define LUAWRAPPER_MEMBER_WRAPPER_SHARED_PTR_EX(RETVAL,NAME,...) LUAWRAPPER_BEGIN_MEMBER_WRAPPER(#NAME)\
        return AutoWrapSharedPtr<RETVAL, classtype, ##__VA_ARGS__> (L, &classtype::NAME);\
        LUAWRAPPER_END_MEMBER_WRAPPER()

#define LUAWRAPPER_DESTRUCTOR_WRAPPER() LUAWRAPPER_BEGIN_METAFUNCTION_WRAPPER("__gc")\
	return luawrapper::AutoDestructor<classtype> (L);\
	LUAWRAPPER_END_METAFUNCTION_WRAPPER()

#define LUAWRAPPER_DESTRUCTOR_WRAPPER_SHARED_PTR() LUAWRAPPER_BEGIN_METAFUNCTION_WRAPPER("__gc")\
	return luawrapper::AutoDestructor<std::shared_ptr<classtype>> (L);\
	LUAWRAPPER_END_METAFUNCTION_WRAPPER()

#define LUAWRAPPER_END_CLASS_WRAPPER() }}}
#endif /* !defined LUAWRAPPER_HPP */
