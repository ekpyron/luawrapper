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

// strong reference to a lua object
class Reference {
public:
    Reference (lua_State *L, const int &index);
    Reference (void) : L (nullptr), ref (LUA_NOREF) {}
    Reference (Reference &&r) : L (r.L), ref (r.ref) {
        r.L = nullptr; r.ref = LUA_NOREF;
    }
    Reference (const Reference &r);
    Reference &operator= (const Reference &r);
    Reference &operator= (Reference &&r);
    ~Reference (void);
    bool valid (void) const {
        return L != nullptr && ref != LUA_NOREF && ref != LUA_REFNIL;
    }
    operator const int &(void) const { return ref; }
    template<typename T>
    T *convert (void);
    void reset (void);
private:
    lua_State *L;
    int ref;
    friend class WeakReference;
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
    WeakReference &operator= (WeakReference &&r);
    ~WeakReference (void);
    bool valid (void) const;
    operator const int &(void) const { return ref; }
    template<typename T>
    T *convert (void);
    void reset (void);
private:
    lua_State *L;
    int ref;
};

namespace detail {

// private helper functions
void AddToTables (lua_State *L, const function *ptr, const size_t &size);
void AddToStaticTables (lua_State *L, const function *ptr, const size_t &size);
void CreateMetatable (lua_State *L, const functionlist &functions);
void RegisterClass (lua_State *L, const char *name, const functionlist &functions);
// template helpers
template<typename T>
struct remove_ptr {
    typedef typename std::remove_pointer<T>::type type;
};
template<typename C>
struct baretype {
    typedef typename std::remove_cv<typename std::remove_reference<C>::type>::type type;
};
template<typename T>
struct has_lua_functions {
private:
    template<typename> struct int_ { typedef int type; };
    template<typename C, typename int_<decltype(detail::remove_ptr<typename detail::baretype<C>::type>::type::lua_functions)>::type = 0>
    static char test (int*t=0);
    template<typename C>
    static long test (...);
public:
    static constexpr bool value = sizeof (test<T>(0)) == sizeof (char);
};
template<typename T>
void CreateMetatable (typename std::enable_if<detail::has_lua_functions<T>::value, lua_State>::type *L) {
    CreateMetatable (L, detail::remove_ptr<typename detail::baretype<T>::type>::type::lua_functions);
}
template<typename T>
void CreateMetatable (typename std::enable_if<!detail::has_lua_functions<T>::value, lua_State>::type *L) {
    CreateMetatable (L, Functions<typename detail::remove_ptr<typename detail::baretype<T>::type>::type>::lua_functions);
}

template<typename T>
struct IsUserdata {
    static constexpr bool value = !std::is_arithmetic<T>::value && !std::is_same<T, ManualReturn>::value
                                  && !std::is_same<T, std::string>::value && !std::is_same<T, Reference>::value
                                  && !std::is_same<T, WeakReference>::value;
};
template<typename R> struct ReturnType { typedef R &type; };
template<> struct ReturnType<std::string> { typedef std::string type; };
template<> struct ReturnType<float> { typedef float type; };
template<> struct ReturnType<double> { typedef double type; };
template<> struct ReturnType<int> { typedef int type; };
template<> struct ReturnType<unsigned int> { typedef unsigned int type; };
template<> struct ReturnType<long> { typedef long type; };
template<> struct ReturnType<unsigned long> { typedef unsigned long type; };
template<> struct ReturnType<bool> { typedef bool type; };
template<> struct ReturnType<lua_State*> { typedef lua_State *type; };
template<> struct ReturnType<Reference> { typedef Reference type; };
template<> struct ReturnType<WeakReference> { typedef WeakReference type; };
inline bool alltrue (void) { return true; }
template<typename... Args> bool alltrue (bool u, Args... args) { return u && alltrue (args...); }
template<int ...> struct seq { };
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> { };
template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };
template <int N, typename... U> struct tuple_element;
template <typename U0, typename... U> struct tuple_element<0, U0, U...> { typedef U0 type; };
template <int N, typename U0, typename... U> struct tuple_element<N, U0, U...> {
    typedef typename tuple_element<N-1, U...>::type type;
};

// dummy datatypes for function description constuctors
struct MetafunctionType {};
struct StaticfunctionType {};
struct ConstructorType {};
struct DestructorType {};
struct IndexfunctionType {};
struct NewindexfunctionType {};

} /* namespace detail */

// dummy values for function description constructors
static detail::MetafunctionType MetaFunction;
static detail::StaticfunctionType StaticFunction;
static detail::ConstructorType Constructor;
static detail::DestructorType Destructor;
static detail::IndexfunctionType IndexFunction;
static detail::NewindexfunctionType NewIndexFunction;

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
    friend void detail::AddToStaticTables (lua_State *L, const function *ptr, const size_t &size);
    friend void detail::AddToTables (lua_State *L, const function *ptr, const size_t &size);
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
        };
    };
};

// type checking on lua stack
template<typename T> inline bool checkarg (lua_State *L, const int &index) {
    // TODO: perform type checking here
    return lua_isuserdata (L, index);
}
template<> inline bool checkarg<float> (lua_State *L, const int &index) { return lua_isnumber (L, index); }
template<> inline bool checkarg<double> (lua_State *L, const int &index) { return lua_isnumber (L, index); }
template<> inline bool checkarg<bool> (lua_State *L, const int &index) { return lua_isboolean (L, index); }
template<> inline bool checkarg<long> (lua_State *L, const int &index) { return lua_isnumber (L, index); }
template<> inline bool checkarg<unsigned long> (lua_State *L, const int &index) { return lua_isnumber (L, index); }
template<> inline bool checkarg<int> (lua_State *L, const int &index) { return lua_isnumber (L, index); }
template<> inline bool checkarg<unsigned int> (lua_State *L, const int &index) { return lua_isnumber (L, index); }
template<> inline bool checkarg<std::string> (lua_State *L, const int &index) { return lua_isstring (L, index); }
template<> inline bool checkarg<lua_State*> (lua_State *L,const int &index) { return true; }
template<> inline bool checkarg<Reference> (lua_State *L, const int &index) { return true; }
template<> inline bool checkarg<WeakReference> (lua_State *L, const int &index) { return true; }

// pulling from the lua stack
template<typename T> inline typename detail::ReturnType<T>::type pull (lua_State *L, const int &index) {
    // TODO: this will NOT work if the userdata was pushed as a pointer
    return *static_cast<T*> (lua_touserdata (L, index));
}
template<> inline float pull<float> (lua_State *L, const int &index) { return lua_tonumber (L, index); }
template<> inline double pull<double> (lua_State *L, const int &index) { return lua_tonumber (L, index); }
template<> inline bool pull<bool> (lua_State *L, const int &index) { return lua_toboolean (L, index); }
template<> inline long pull<long> (lua_State *L, const int &index) { return lua_tointeger (L, index); }
template<> inline unsigned long pull<unsigned long> (lua_State *L, const int &index) { return lua_tointeger (L, index); }
template<> inline int pull<int> (lua_State *L, const int &index) { return lua_tointeger (L, index); }
template<> inline unsigned int pull<unsigned int> (lua_State *L, const int &index) { return lua_tointeger (L, index); }
template<> inline std::string pull<std::string> (lua_State *L, const int &index) { size_t len = 0;
    const char *str = lua_tolstring (L, index, &len);
    return std::string (str, len);
}
template<> inline Reference pull<Reference> (lua_State *L, const int &index) {
    return Reference (L, index);
}
template<> inline WeakReference pull<WeakReference> (lua_State *L, const int &index);
template<> inline lua_State *pull<lua_State *> (lua_State *L, const int &index) { return L; }

// pushing to lua stack
template<typename T, typename... Args>
void push (typename std::enable_if<detail::IsUserdata<T>::value && !std::is_pointer<T>::value, lua_State>::type *L, Args... args) {
    T *obj = static_cast<T*> (lua_newuserdata (L, sizeof (T)));
    detail::CreateMetatable<T> (L);
    // construct object
    new (obj) T (args...);
    // set metatable for userdata, hence ensuring destruction of the object
    lua_setmetatable (L, -2);
}
template<typename T>
void push (typename std::enable_if<detail::IsUserdata<T>::value && std::is_pointer<T>::value, lua_State>::type *L, const T &t) {
    T *obj = static_cast<T*> (lua_newuserdata (L, sizeof (T)));
    *obj = t;
    lua_pushlightuserdata (L, t);
    detail::CreateMetatable<T> (L);
    lua_remove (L, -2);
    lua_setmetatable (L, -2);
}
template<typename T> void push (typename std::enable_if<!detail::IsUserdata<T>::value, lua_State>::type *L, const T &t);
template<> inline void push<float> (lua_State *L, const float &v) { lua_pushnumber (L, v); }
template<> inline void push<double> (lua_State *L, const double &v) { lua_pushnumber (L, v); }
template<> inline void push<bool> (lua_State *L, const bool &v) { lua_pushboolean (L, v); }
template<> inline void push<long> (lua_State *L, const long &v) { lua_pushinteger (L, v); }
template<> inline void push<unsigned long> (lua_State *L, const unsigned long &v) { lua_pushinteger (L, v); }
template<> inline void push<int> (lua_State *L, const int &v) { lua_pushinteger (L, v); }
template<> inline void push<unsigned int> (lua_State *L, const unsigned int &v) { lua_pushinteger (L, v); }
template<> inline void push<std::string> (lua_State *L, const std::string &v) { lua_pushlstring (L, v.data (), v.length ()); }
template<> inline void push<ManualReturn> (lua_State *L, const ManualReturn &v) { }
template<> inline void push<Reference> (lua_State *L, const Reference &v) { lua_rawgeti (L, LUA_REGISTRYINDEX, v); }
template<> inline void push<WeakReference> (lua_State *L, const WeakReference &v);

namespace detail {

// Call helper for functions with return value.
template<typename Retval, typename T, typename... Args>
struct Caller {
private:
    template<int N>
    struct C {
        typedef typename ReturnType<typename detail::baretype<typename tuple_element<N, Args...>::type>::type>::type rettype;
        typedef typename detail::baretype<typename tuple_element<N, Args...>::type>::type baretype;
    };
    template<int N>
    static bool check (lua_State *L, int startindex) {
        return checkarg<typename C<N>::baretype> (L, startindex + N);
    }
    template<int N>
    static typename C<N>::rettype get (lua_State *L, int startindex) {
        return pull<typename C<N>::baretype> (L, startindex + N);
    }
    template<int ...S>
    static Retval call_helper (lua_State *L, int startindex, T *t, Retval (T::*fn) (Args...), seq<S...>) {
        return (t->*fn) (get<S> (L, startindex)...);
    }
    template<int ...S>
    static Retval call_helper (lua_State *L, int startindex, T *t, Retval (T::*fn) (Args...) const, seq<S...>) {
        return (t->*fn) (get<S> (L, startindex)...);
    }
public:
    static typename std::enable_if<!std::is_void<T>::value, bool>::type try_call (lua_State *L, int &results, int startindex, T *t, Retval (T::*fn) (Args...)) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return false;
        push<typename detail::baretype<Retval>::type> (L, call_helper (L, startindex, t, fn, typename gens<sizeof...(Args)>::type ()));
        results = 1;
        return true;
    }
    static typename std::enable_if<!std::is_void<T>::value, bool>::type try_call (lua_State *L, int &results, int startindex, T *t, Retval (T::*fn) (Args...) const) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return false;
        push<typename detail::baretype<Retval>::type> (L, call_helper (L, startindex, t, fn, typename gens<sizeof...(Args)>::type ()));
        results = 1;
        return true;
    }
    static typename std::enable_if<!std::is_void<T>::value, int>::type call (lua_State *L, int startindex, T *t, Retval (T::*fn) (Args...)) {
        int results;
        if (!try_call (L, results, startindex, t, fn))
            luaL_error (L, "Invalid arguments.");
        return results;
    }
    static typename std::enable_if<!std::is_void<T>::value, int>::type call (lua_State *L, int startindex, T *t, Retval (T::*fn) (Args...) const) {
        int results;
        if (!try_call (L, results, startindex, t, fn))
            luaL_error (L, "Invalid arguments.");
        return results;
    }
private:
    template<int ...S>
    static bool checkargs (lua_State *L, int startindex, seq<S...>) {
        if (lua_gettop (L) != startindex + sizeof... (Args) - 1) return false;
        return alltrue (check<S> (L, startindex)...);
    }
};

// Call helper for functions without return value.
template<typename T, typename... Args>
struct Caller<void, T, Args...>
{
private:
    template<int N>
    struct C {
        typedef typename ReturnType<typename detail::baretype<typename tuple_element<N, Args...>::type>::type>::type rettype;
        typedef typename detail::baretype<typename tuple_element<N, Args...>::type>::type baretype;
    };
    template<int N>
    static bool check (lua_State *L, int startindex) {
        return checkarg<typename C<N>::baretype> (L, startindex + N);
    }
    template<int N>
    static typename C<N>::rettype get (lua_State *L, int startindex) {
        return pull<typename C<N>::baretype> (L, startindex + N);
    }
    template<int ...S>
    static void call_helper (lua_State *L, int startindex, T *t, void (T::*fn) (Args...), seq<S...>) {
        return (t->*fn) (get<S> (L, startindex)...);
    }
    template<int ...S>
    static void call_helper (lua_State *L, int startindex, T *t, void (T::*fn) (Args...) const, seq<S...>) {
        (t->*fn) (get<S> (L, startindex)...);
    }
public:
    static bool try_call (lua_State *L, int &results, int startindex, T *t, void (T::*fn) (Args...)) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return false;
        results = 0;
        call_helper (L, startindex, t, fn, typename gens<sizeof...(Args)>::type ());
        return true;
    }
    static bool try_call (lua_State *L, int &results, int startindex, T *t, void (T::*fn) (Args...) const) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return false;
        results = 0;
        call_helper (L, startindex, t, fn, typename gens<sizeof...(Args)>::type ());
        return true;
    }
    static int call (lua_State *L, int startindex, T *t, void (T::*fn) (Args...)) {
        int results;
        if (!try_call (L, results, startindex, t, fn))
            luaL_error (L, "Invalid arguments.");
        return results;
    }
    static int call (lua_State *L, int startindex, T *t, void (T::*fn) (Args...) const) {
        int results;
        if (!try_call (L, results, startindex, t, fn))
            luaL_error (L, "Invalid arguments.");
        return results;
    }
private:
    template<int ...S>
    static bool checkargs (lua_State *L, int startindex, seq<S...>) {
        if (lua_gettop (L) != startindex + sizeof... (Args) - 1) return false;
        return alltrue (check<S> (L, startindex)...);
    }
};

// Call helper for constructors.
template<typename T, typename... Args>
struct Constructor {
private:
    template<int N>
    struct C {
        typedef typename ReturnType<typename detail::baretype<typename tuple_element<N, Args...>::type>::type>::type rettype;
        typedef typename detail::baretype<typename tuple_element<N, Args...>::type>::type baretype;
    };
    template<int N>
    static bool check (lua_State *L, int startindex) {
        return checkarg<typename C<N>::baretype> (L, startindex + N);
    }
    template<int N>
    static typename C<N>::rettype get (lua_State *L, int startindex) {
        return pull<typename C<N>::baretype> (L, startindex + N);
    }
    template<int ...S>
    static void construct_helper (lua_State *L, int startindex, void *ptr, seq<S...>) {
        new (ptr) T (get<S> (L, startindex)...);
    }
    template<int ...S>
    static T *construct_helper (lua_State *L, int startindex, seq<S...>) {
        return new T (get<S> (L, startindex)...);
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
    static bool try_construct (lua_State *L, int startindex, T *ptr) {
        if (!checkargs (L, startindex, typename gens<sizeof...(Args)>::type ())) return false;
        construct_helper (L, startindex, ptr, typename gens<sizeof...(Args)>::type ());
        return true;
    }
    static void construct (lua_State *L, int startindex, T *ptr) {
        if (!try_construct (L, startindex, ptr))
            luaL_error (L, "Invalid arguments.");
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
    static int Wrap(lua_State *L) {
        T *ptr = static_cast<T *> (lua_newuserdata(L, sizeof(T)));
        lua::detail::Constructor<T, Args...>::construct(L, 2, ptr);
        lua::detail::CreateMetatable(L, detail::remove_ptr<T>::type::lua_functions);
        lua_setmetatable(L, -2);
        return 1;
    }
    static bool Wrap(lua_State *L, int &results) {
        T *ptr = static_cast<T *> (lua_newuserdata(L, sizeof(T)));
        if (!lua::detail::Constructor<T, Args...>::try_construct(L, 2, ptr)) {
            lua_pop (L, 1);
            return false;
        }
        lua::detail::CreateMetatable(L, detail::remove_ptr<T>::type::lua_functions);
        lua_setmetatable(L, -2);
        results = 1;
        return true;
    }
};

// Destructor wrappers.
template<typename T>
int Destruct (lua_State *L) {
    T *obj = static_cast<T*> (lua_touserdata (L, lua_upvalueindex (1)));
    obj->~T ();
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
    friend WeakReference pull<WeakReference> (lua_State *L, const int &index);
    friend void push<WeakReference> (lua_State *L, const WeakReference &v);
};

// Class registration.
template<typename T>
void register_class (lua_State *L, const char *name, const functionlist &functions = T::lua_functions) {
    detail::RegisterClass (L, name, functions);
}

// member implementations for references
template<typename T>
T *Reference::convert (void) {
    if (L == nullptr || ref == LUA_NOREF || ref == LUA_REFNIL) return nullptr;
    lua_rawgeti (L, LUA_REGISTRYINDEX, ref);
    if (!checkarg<T> (L, -1)) {
        lua_pop (L, 1);
        return nullptr;
    }
    T *ptr = static_cast<T*> (lua_touserdata (L, -1));
    lua_pop (L, 1);
    return ptr;
}

template<typename T>
T *WeakReference::convert (void) {
    if (L == nullptr || ref == LUA_NOREF || ref == LUA_REFNIL) return nullptr;
    State::push_weak_registry (L);
    lua_rawgeti (L, -1, ref);
    if (!checkarg<T> (L, -1)) {
        lua_pop (L, 2);
        return nullptr;
    }
    T *ptr = static_cast<T*> (lua_touserdata (L, -1));
    lua_pop (L, 2);
    return ptr;
}

template<> inline WeakReference pull<WeakReference> (lua_State *L, const int &index) {
    return WeakReference (L, index);
}

template<> inline void push<WeakReference> (lua_State *L, const WeakReference &v) {
    State::push_weak_registry (L);
    lua_rawgeti (L, -1, v);
    lua_remove (L, -2);
}

} /* namespace lua */

#endif /* LUAWRAPPER_H */
