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
#ifndef LUAWRAPPER_H
#define LUAWRAPPER_H

#include <vector>
#include <stdexcept>
#include <typeinfo>
#include <type_traits>
#include <limits>
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace lua {

class function;
typedef std::initializer_list<function> functionlist;
struct ManualReturn {};
template<typename T>
struct Functions;

class Reference;
class WeakReference;
template<typename T>
class TypedReference;

namespace detail {

// private helper functions
void AddToTables (lua_State *L, const function *ptr, const size_t &size, std::vector<size_t> &typehashs) noexcept;
void AddToStaticTables (lua_State *L, const function *ptr, const size_t &size) noexcept;
bool CheckType (lua_State *L, const int &index, const size_t &typehash);
void CreateMetatable (lua_State *L, const functionlist &functions, const size_t &typehash) noexcept;
template<typename T>
void CreateMetatable (lua_State *L) noexcept {
    CreateMetatable (L, Functions<T>::value, typeid (T).hash_code ());
}
void register_class (lua_State *L, const char *name, const functionlist &functions);

// template helpers
template<typename T>
using baretype = std::remove_cv<typename std::remove_reference<T>::type>;
template<int ...> struct seq { };
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> { };
template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };
template <int N, typename... U> struct tuple_element;
template <typename U0, typename... U> struct tuple_element<0, U0, U...> { typedef U0 type; };
template <int N, typename U0, typename... U> struct tuple_element<N, U0, U...> {
    typedef typename tuple_element<N-1, U...>::type type;
};
inline bool alltrue (void) { return true; }
template<typename... Args> bool alltrue (bool u, Args... args) { return u && alltrue (args...); }

// dummy datatypes for function description constructors
struct MetafunctionType {};
struct StaticfunctionType {};
struct ConstructorType {};
struct DestructorType {};
struct IndexfunctionType {};
struct NewindexfunctionType {};
template<typename T>
struct BaseClassType {};


} /* namespace detail */

template<typename T, class = void>
struct Type;

// strong reference to a lua object
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
    template<typename T>
    T *convert (void) const;
    template<typename T>
    bool checktype (void) const;
    void reset (void);
    lua_State* const &GetLuaState (void) const { return L; }
    void push (void) const;
    bool operator< (const Reference &r) const;
    bool operator== (const Reference &r) const;
private:
    lua_State *L;
    int ref;
    void *ptr;
    friend class WeakReference;
    template<typename T>
    friend class TypedReference;
    template<typename, class>
    friend struct Type;
};

// typed reference to a lua object
template<typename T>
class TypedReference : public Reference {
public:
    TypedReference (lua_State *L, const int &index) : Reference (L, index) {
        if (!checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
    }
    TypedReference (void) : Reference () {
    }
    TypedReference (TypedReference<T> &&r) : Reference (r) {
        if (!checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
    }
    TypedReference (const TypedReference<T> &r) : Reference (r) {
        if (!checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
    }
    TypedReference (Reference &&r) : Reference (r) {
        if (!checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
    }
    TypedReference (const Reference &r) : Reference (r) {
        if (!checktype<T> ()) throw std::runtime_error ("Lua value has invalid type.");
    }
    operator T* (void) const {
        return static_cast<T*> (ptr);
    }
    T *operator-> (void) {
        return static_cast<T*> (ptr);
    }
    const T *operator-> (void) const {
        return static_cast<const T*> (ptr);
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

// weak reference to a lua object
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
    T *convert (void) const;
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

// function fetcher
template<typename T>
struct Functions : std::integral_constant<functionlist&, std::remove_pointer<typename detail::baretype<T>::type>::type::lua_functions> {
};

// pushing to lua stack
template<typename T, typename... Args>
T *push (typename std::enable_if<!std::is_pointer<T>::value, lua_State>::type *L, Args... args) {
    // construct object
    T *obj = new T (args...);
    // the following functions don't throw
    *static_cast<T**> (lua_newuserdata (L, sizeof (T*))) = obj;
    lua_pushlightuserdata (L, obj);
    detail::CreateMetatable<T> (L);
    // set metatable for userdata, hence ensuring destruction of the object
    lua_setmetatable (L, -3);
    lua_pop (L, 1);
    return obj;
}
template<typename T>
T *push (typename std::enable_if<std::is_pointer<T>::value, lua_State>::type *L, const T &t) {
    T *obj = static_cast<T*> (lua_newuserdata (L, sizeof (T)));
    *obj = t;
    lua_pushlightuserdata (L, t);
    detail::CreateMetatable<T> (L);
    lua_setmetatable (L, -3);
    lua_pop (L, 1);
    return obj;
}

// supported types
template<typename T, class>
struct Type
{
    static T &pull (lua_State *L, const int &index) {
        return **static_cast<T**> (lua_touserdata (L, index));
    }
    static bool check (lua_State *L, const int &index) {
        return lua_isnil (L, index) || lua_isuserdata (L, index) && detail::CheckType (L, index, typeid (T).hash_code ());
    }
    static void push (lua_State *L, T &&v) {
        lua::push (L, std::move (v));
    }
};

template<typename T>
struct Type<T, typename std::enable_if<std::is_pointer<T>::value>::type>
{
    static T &pull (lua_State *L, const int &index) {
        return *static_cast<T*> (lua_touserdata (L, index));
    }
    static bool check (lua_State *L, const int &index) {
        return lua_isnil (L, index) || lua_isuserdata (L, index) && detail::CheckType (L, index, typeid (T).hash_code ());
    }
    static void push (lua_State *L, T &&v) {
        lua::push (L, std::move (v));
    }
};

template<typename T>
struct Type<T, typename std::enable_if<std::numeric_limits<T>::is_integer>::type>
{
    static T pull (lua_State *L, const int &index) {
        return lua_tointeger (L, index);
    }
    static bool check (lua_State *L, const int &index) {
        return lua_isnumber (L, index);
    }
    static void push (lua_State *L, const T &v) {
        lua_pushinteger (L, v);
    }
};

template<typename T>
struct Type<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
{
    static void push (lua_State *L, const T &v) { lua_pushnumber (L, v); }
    static T pull (lua_State *L, const int &index) { return lua_tonumber (L, index); }
    static bool check (lua_State *L, const int &index) { return lua_isnumber (L, index); }
};

template<>
struct Type<lua_State*> {
    static lua_State *pull (lua_State *L, const int &index) { return L; }
    static bool check (lua_State *L, const int &index) { return true; }
};

template<>
struct Type<bool> {
    static bool check (lua_State *L, const int &index) { return lua_isboolean (L, index); }
    static bool pull (lua_State *L, const int &index) { return lua_toboolean (L, index); }
    static void push (lua_State *L, const bool &v) { lua_pushboolean (L, v); }
};

template<>
struct Type<std::string>
{
    static bool check (lua_State *L, const int &index) { return lua_isstring (L, index); }
    static std::string pull (lua_State *L, const int &index) {
        size_t len = 0;
        const char *str = lua_tolstring (L, index, &len);
        return std::string (str, len);
    }
    static void push (lua_State *L, const std::string &v) { lua_pushlstring (L, v.data (), v.length ()); }
};

template<>
struct Type<ManualReturn>
{
    static void push (lua_State *L, const ManualReturn &v) { }
};

template<>
struct Type<Reference>
{
    static bool check (lua_State *L, const int &index) { return true; }
    static Reference pull (lua_State *L, const int &index) {
        return Reference (L, index);
    }
    static void push (lua_State *L, const Reference &t) {
        lua_rawgeti (L, LUA_REGISTRYINDEX, t.ref);
    }
};

template<typename T>
struct Type<TypedReference<T>>
{
    static bool check (lua_State *L, const int &index) { return true; }
    static TypedReference<T> pull (lua_State *L, const int &index) {
        return TypedReference<T> (L, index);
    }
    static void push (lua_State *L, const TypedReference<T> &t) {
        lua_rawgeti (L, LUA_REGISTRYINDEX, t.ref);
    }
};

template<>
struct Type<WeakReference>
{
    static bool check (lua_State *L, const int &index) { return true; }
    static WeakReference pull (lua_State *L, const int &index);
    static void push (lua_State *L, const WeakReference &v);
};

// dummy values for function description constructors
static detail::MetafunctionType MetaFunction;
static detail::StaticfunctionType StaticFunction;
static detail::ConstructorType Constructor;
static detail::DestructorType Destructor;
static detail::IndexfunctionType IndexFunction;
static detail::NewindexfunctionType NewIndexFunction;

template<typename T>
detail::BaseClassType<T> BaseClass (void) {}

// function description
class function {
public:
    function (const char *_name, const lua_CFunction &_func)
            : type (MEMBERFUNCTION), func (_func), name (_name) {
    }
    function (const char *_name, const lua_CFunction &_func, detail::MetafunctionType)
            : type (METAFUNCTION), func (_func), name (_name) {
    }
    function (const char *_name, const lua_CFunction &_func, detail::StaticfunctionType)
            : type (STATICFUNCTION), func (_func), name (_name) {
    }
    function (const lua_CFunction &_func, detail::ConstructorType)
            : type (CONSTRUCTOR), func (_func), name (nullptr) {
    }
    function (const lua_CFunction &_func, detail::DestructorType)
            : type (METAFUNCTION), func (_func), name ("__gc") {
    }
    function (const lua_CFunction &_func, detail::IndexfunctionType)
            : type (INDEXFUNCTION), func (_func), name (nullptr) {
    }
    function (const lua_CFunction &_func, detail::NewindexfunctionType)
            : type (METAFUNCTION), func (_func), name ("__newindex") {
    }
    function (const functionlist &functions) : type (BASECLASS), ptr (functions.begin ()), size (functions.size ()) {
    }
    template<typename T>
    function (detail::BaseClassType<T> (*func) (void)) : type (BASECLASS), ptr (Functions<T>::value.begin ()),
                                                         size (Functions<T>::value.size ()), hashcode (typeid (T).hash_code ()) {

    }
    friend void detail::AddToStaticTables (lua_State *L, const function *ptr, const size_t &size) noexcept;
    friend void detail::AddToTables (lua_State *L, const function *ptr, const size_t &size, std::vector<size_t> &typehashs) noexcept;
private:
    enum Type {
        MEMBERFUNCTION,
        METAFUNCTION,
        STATICFUNCTION,
        BASECLASS,
        CONSTRUCTOR,
        INDEXFUNCTION
    };
    enum Type type;
    union {
        struct {
            const char *name;
            lua_CFunction func;
        };
        struct {
            const function *ptr;
            size_t size;
            size_t hashcode;
        };
    };
};

namespace detail {

// Call helper for functions.
template<typename Retval, typename T, typename... Args>
struct Caller {
private:
    template<int N>
    using argtype = Type<typename detail::baretype<typename tuple_element<N, Args...>::type>::type>;
    using rettype = Type<typename detail::baretype<Retval>::type>;
    template<int N>
    static bool check (lua_State *L, int startindex) {
        return argtype<N>::check (L, startindex + N);
    }
    template<int N>
    static decltype (argtype<N>::pull (nullptr, 0)) get (lua_State *L, int startindex) {
        return argtype<N>::pull (L, startindex + N);
    }
    template<typename R, typename FN, int ...S>
    static int do_call (typename std::enable_if<!std::is_same<R, void>::value, lua_State>::type *L,
                        int startindex, T *t, FN fn, seq<S...>) {
        rettype::push (L, (t->*fn) (get<S> (L, startindex)...));
        return 1;
    }
    template<typename R, typename FN, int ...S>
    static int do_call (typename std::enable_if<std::is_same<R, void>::value, lua_State>::type *L,
                        int startindex, T *t, FN fn, seq<S...>) {
        (t->*fn) (get<S> (L, startindex)...);
        return 0;
    }
    template<int ...S>
    static bool checkargs (lua_State *L, int startindex, seq<S...>) {
        if (lua_gettop (L) != startindex + sizeof... (Args) - 1) return false;
        return alltrue (check<S> (L, startindex)...);
    }
public:
    template<typename FN>
    static bool try_call (lua_State *L, int &results, int startindex, T *t, FN fn) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return false;
        try {
            results = do_call<Retval> (L, startindex, t, fn, typename gens<sizeof...(Args)>::type ());
        } catch (std::exception &e) {
            luaL_error (L, "Lua error: %s", e.what ());
        } catch (...) {
            luaL_error (L, "Lua error: unknown exception.");
        }
        return true;
    }
    template<typename FN>
    static int call (lua_State *L, int startindex, T *t, FN fn) {
        int results;
        if (!try_call (L, results, startindex, t, fn))
            luaL_error (L, "Invalid arguments.");
        return results;
    }
};

// Call helper for constructors.
template<typename T, typename... Args>
struct Constructor {
private:
    template<int N>
    using argtype = Type<typename detail::baretype<typename tuple_element<N, Args...>::type>::type>;
    template<int N>
    static bool check (lua_State *L, int startindex) {
        return argtype<N>::check (L, startindex + N);
    }
    template<int N>
    static decltype (argtype<N>::pull (nullptr, 0)) get (lua_State *L, int startindex) {
        return argtype<N>::pull (L, startindex + N);
    }
    template<int ...S>
    static T *construct_helper (lua_State *L, int startindex, seq<S...>) {
        try {
            return new T (get<S> (L, startindex)...);
        } catch (std::exception &e) {
            luaL_error (L, "Lua error: %s", e.what ());
        } catch (...) {
            luaL_error (L, "Lua error: unknown exception.");
        }
    }
    template<int ...S>
    static bool checkargs (lua_State *L, int startindex, seq<S...>) {
        if (lua_gettop (L) != startindex + sizeof... (Args)) return false;
        return alltrue (check<S> (L, startindex)...);
    }
public:
    static T *try_construct (lua_State *L, int startindex) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return nullptr;
        return construct_helper (L, startindex, typename gens<sizeof...(Args)>::type ());
    }
    static T *construct (lua_State *L, int startindex) {
        T *t = try_construct (L, startindex);
        if (t == nullptr) luaL_error (L, "Invalid arguments.");
        return t;
    }
};

// Overload helper
template<class... Args>
struct Overload;
template<>
struct Overload<> {
    static bool Function (lua_State *L, int &results) { return false; }
    static bool Construct (lua_State *L, int &results) { return false; }
};
template<class C, class... Args>
struct Overload<C, Args...> {
    static bool Function (lua_State *L, int &results) {
        return C::Wrap (L, results) || Overload<Args...>::Function (L, results);
    }
    static bool Construct (lua_State *L, int &results) {
        return C::Wrap (L, results) || Overload<Args...>::Construct (L, results);
    }
};

} /* namespace detail */

// Constructor wrappers.
template<typename T, typename... Args>
struct Construct {
    static int Wrap (lua_State *L) {
        T **ptr = static_cast<T**> (lua_newuserdata (L, sizeof (T*)));
        *ptr = lua::detail::Constructor<T, Args...>::construct (L, 2);
        lua_pushlightuserdata (L, *ptr);
        lua::detail::CreateMetatable<T> (L);
        lua_setmetatable (L, -3);
        lua_pop (L, 1);
        return 1;
    }

    static bool Wrap (lua_State *L, int &results) {
        T **ptr = static_cast<T**> (lua_newuserdata (L, sizeof (T*)));
        *ptr = lua::detail::Constructor<T, Args...>::try_construct (L, 2);
        if (*ptr == nullptr) {
            lua_pop (L, 1);
            return false;
        }
        lua_pushlightuserdata (L, *ptr);
        lua::detail::CreateMetatable<T> (L);
        lua_setmetatable (L, -3);
        lua_pop (L, 1);
        results = 1;
        return true;
    }
};
template<typename T, typename... Args>
struct ConstructWithSelfReference {
    static int Wrap (lua_State *L) {
        T **ptr = static_cast<T**> (lua_newuserdata(L, sizeof(T*)));
        lua_pushvalue (L, -1);
        lua_insert (L, 2);
        *ptr = lua::detail::Constructor<T, Reference, Args...>::construct(L, 2);
        lua_pushlightuserdata (L, *ptr);
        lua::detail::CreateMetatable<T> (L);
        lua_setmetatable(L, -3);
        lua_pop (L, 1);
        return 1;
    }
    static bool Wrap (lua_State *L, int &results) {
        T **ptr = static_cast<T**> (lua_newuserdata(L, sizeof(T*)));
        lua_pushvalue (L, -1);
        lua_insert (L, 2);
        *ptr = lua::detail::Constructor<T, Reference, Args...>::try_construct(L, 2);
        if (*ptr == nullptr) {
            lua_pop (L, 1);
            return false;
        }
        lua_pushlightuserdata (L, *ptr);
        lua::detail::CreateMetatable<T> (L);
        lua_setmetatable(L, -3);
        lua_pop (L, 1);
        results = 1;
        return true;
    }
};

// Destructor wrappers.
template<typename T>
int Destruct (lua_State *L) noexcept {
    T *obj = static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1)));
    delete obj;
    return 0;
}

// Function wrappers.
template<typename Retval, typename... Args>
struct Function;
template<typename Retval, typename... Args>
struct Function<Retval(Args...)> {
    template<typename T, Retval (T::*M) (Args...), int skipargs = 0>
    struct Overload {
        static bool Wrap (lua_State *L, int &results) {
            T *t = static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1)));
            return lua::detail::Caller<Retval, T, Args...>::try_call (L, results, 1 + skipargs, t, M);
        }
    };
    template<typename T, Retval (T::*M) (Args...), int skipargs = 0>
    static int Wrap (lua_State *L) {
        T *t = static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1)));
        return lua::detail::Caller<Retval, T, Args...>::call (L, 1 + skipargs, t, M);
    }
};
template<typename Retval, typename... Args>
struct Function<Retval(Args...)const> {
    template<typename T, Retval (T::*M) (Args...) const, int skipargs = 0>
    struct Overload {
        static bool Wrap (lua_State *L, int &results) {
            T *t = static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1)));
            return lua::detail::Caller<Retval, T, Args...>::try_call (L, results, 1 + skipargs, t, M);
        }
    };
    template<typename T, Retval (T::*M) (Args...) const, int skipargs = 0>
    static int Wrap (lua_State *L) {
        T *t = static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1)));
        return lua::detail::Caller<Retval, T, Args...>::call (L, 1 + skipargs, t, M);
    }
};

// Overload wrappers.
template<class... Args>
struct Overload {
    static int Function (lua_State *L) {
        int results;
        if (!detail::Overload<Args...>::Function (L, results))
            luaL_error (L, "Invalid arguments.");
        return results;
    }
    static int Construct (lua_State *L) {
        int results;
        if (!detail::Overload<Args...>::Construct (L, results))
            luaL_error (L, "Invalid arguments.");
        return results;
    }
};

// Lua State wrapper.
class State {
public:
    State (void);
    State (const State&) = delete;
    ~State (void);
    State &operator= (const State&) = delete;
    operator lua_State * (void) const {
        return L;
    }
    void loadlib (const lua_CFunction &fn, const std::string &name);
private:
    static void push_weak_registry (lua_State *L);
    lua_State *L;
    friend class WeakReference;
    template<typename, class>
    friend struct Type;
};

// Class registration.
template<typename T>
void register_class (lua_State *L, const char *name) {
    detail::register_class (L, name, Functions<T>::value);
}

// member implementations for references
template<typename T>
T *Reference::convert (void) const {
    return static_cast<T*> (ptr);
}
template<typename T>
bool Reference::checktype (void) const {
    if (L == nullptr || ref == LUA_NOREF || ref == LUA_REFNIL) return false;
    lua_rawgeti (L, LUA_REGISTRYINDEX, ref);
    bool result = Type<T>::check (L, -1);
    lua_pop (L, 1);
    return result;
}

template<typename T>
T *WeakReference::convert (void) const {
    if (L == nullptr || ref == LUA_NOREF || ref == LUA_REFNIL) return nullptr;
    State::push_weak_registry (L);
    lua_rawgeti (L, -1, ref);
    if (!Type<T>::check (L, -1)) {
        lua_pop (L, 2);
        return nullptr;
    }
    T *ptr = static_cast<T*> (lua_touserdata (L, -1));
    lua_pop (L, 2);
    return ptr;
}

} /* namespace lua */

#endif /* !defined LUAWRAPPER_H */
