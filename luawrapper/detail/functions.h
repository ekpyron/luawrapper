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

template<typename T>
struct Functions : std::integral_constant<functionlist&,
        std::remove_pointer<typename detail::baretype<T>::type>::type::lua_functions> { };

struct ManualReturn { };

namespace detail {

struct MetafunctionType {};
struct StaticfunctionType {};
struct ConstructorType {};
struct DestructorType {};
struct IndexfunctionType {};
struct NewindexfunctionType {};
template<typename T>
struct BaseClassType {};

} /* namespace detail */

// dummy values for function description constructors
static detail::MetafunctionType META_FUNCTION;
static detail::StaticfunctionType STATIC_FUNCTION;
static detail::ConstructorType CONSTRUCTOR;
static detail::DestructorType DESTRUCTOR;
static detail::IndexfunctionType INDEX_FUNCTION;
static detail::NewindexfunctionType NEW_INDEX_FUNCTION;

template<typename T>
detail::BaseClassType<T> BaseClass (void) {}

// function description
class function {
public:
    function (const char *_name, const lua_CFunction &_func)
            : type (MEMBERFUNCTION), func (_func), name (_name) { }
    function (const char *_name, const lua_CFunction &_func, detail::MetafunctionType)
            : type (METAFUNCTION), func (_func), name (_name) { }
    function (const char *_name, const lua_CFunction &_func, detail::StaticfunctionType)
            : type (STATICFUNCTION), func (_func), name (_name) { }
    function (const lua_CFunction &_func, detail::ConstructorType)
            : type (CONSTRUCTOR), func (_func), name (nullptr) { }
    function (const lua_CFunction &_func, detail::DestructorType)
            : type (DESTRUCTOR), func (_func), name (nullptr) { }
    function (const lua_CFunction &_func, detail::IndexfunctionType)
            : type (INDEXFUNCTION), func (_func), name (nullptr) { }
    function (const lua_CFunction &_func, detail::NewindexfunctionType)
            : type (METAFUNCTION), func (_func), name ("__newindex") { }
    template<typename T>
    function (detail::BaseClassType<T> (*func) (void)) : type (BASECLASS), ptr (Functions<T>::value.begin ()),
                                                         size (Functions<T>::value.size ()),
                                                         hashcode (typeid (T).hash_code ()) { }
    friend void detail::AddToStaticTables (lua_State *L, const function *ptr, const size_t &size) noexcept;
    friend void detail::AddToTables (lua_State *L, const function *ptr, const size_t &size, std::vector<size_t> &typehashs, bool destructor) noexcept;
private:
    enum Type {
        MEMBERFUNCTION,
        METAFUNCTION,
        STATICFUNCTION,
        BASECLASS,
        CONSTRUCTOR,
        DESTRUCTOR,
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

} /* namespace lua */
