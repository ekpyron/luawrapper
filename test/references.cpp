#include "common.h"
#include "Object.h"

class Test
{
public:
    static void F1 (lua::Reference ref) {
        check (ref.checktype<int> () && ref.convert<int> () == 201, "integer reference by value");
    }
    static void F2 (const lua::Reference &ref) {
        check (ref.checktype<int> () && ref.convert<int> () == 202, "integer reference by const reference");
    }
    static void F3 (lua::Reference &&ref) {
        check (ref.checktype<int> () && ref.convert<int> () == 203, "integer reference by rvalue reference");
    }
    static void F4 (lua::WeakReference ref) {
        check (ref.checktype<int> () && ref.convert<int> () == 204, "integer weak reference by value");
    }
    static void F5 (const lua::WeakReference &ref) {
        check (ref.checktype<int> () && ref.convert<int> () == 205, "integer weak reference by reference");
    }
    static void F6 (lua::WeakReference &&ref) {
        check (ref.checktype<int> () && ref.convert<int> () == 206, "integer weak reference by rvalue reference");
    }
    static void F7 (lua::TypedReference<int> ref) {
        check (ref == 207, "integer typed reference by value");
    }
    static void F8 (const lua::TypedReference<int> &ref) {
        check (ref == 208, "integer typed reference by const reference");
    }
    static void F9 (lua::TypedReference<int> &&ref) {
        check (ref == 209, "integer typed reference by rvalue reference");
    }
    static void F10 (lua::Reference ref) {
        check (ref.checktype<Object> () && ref.convert<Object*> ()->checkstate (EXPLICIT, false, false, 210), "object reference by value");
    }
    static void F11 (const lua::Reference &ref) {
        check (ref.checktype<Object> () && ref.convert<Object*> ()->checkstate (EXPLICIT, false, false, 211), "object reference by const reference");
    }
    static void F12 (lua::Reference &&ref) {
        check (ref.checktype<Object> () && ref.convert<Object*> ()->checkstate (EXPLICIT, false, false, 212), "object reference by rvalue reference");
    }
    static void F13 (lua::WeakReference ref) {
        check (ref.checktype<Object> () && ref.convert<Object*> ()->checkstate (EXPLICIT, false, false, 213), "object weak reference by value");
    }
    static void F14 (const lua::WeakReference &ref) {
        check (ref.checktype<Object> () && ref.convert<Object*> ()->checkstate (EXPLICIT, false, false, 214), "object weak reference by const reference");
    }
    static void F15 (lua::WeakReference &&ref) {
        check (ref.checktype<Object> () && ref.convert<Object*> ()->checkstate (EXPLICIT, false, false, 215), "object weak reference by rvalue reference");
    }
    static void F16 (lua::TypedReference<Object*> ref) {
        check (ref->checkstate (EXPLICIT, false, false, 216), "object typed reference by value");
    }
    static void F17 (const lua::TypedReference<Object*> &ref) {
        check (ref->checkstate (EXPLICIT, false, false, 217), "object typed reference by const reference");
    }
    static void F18 (lua::TypedReference<Object*> &&ref) {
        check (ref->checkstate (EXPLICIT, false, false, 218), "object typed reference by rvalue reference");
    }
    static lua::Reference F19 (lua_State *L) {
        lua_pushinteger (L, 219);
        lua::Reference ref (L, -1);
        lua_pop (L, 1);
        return ref;
    }
    static void F20 (lua::TypedReference<Object*> ref) {
        check (ref.isnil (), "nil passed as typed reference by value");
    }


    static lua::functionlist lua_functions;
};

lua::functionlist Test::lua_functions = {
        { "F1", lua::Function<void(lua::Reference)>::Wrap<&Test::F1>, lua::STATIC_FUNCTION },
        { "F2", lua::Function<void(const lua::Reference&)>::Wrap<&Test::F2>, lua::STATIC_FUNCTION },
        { "F3", lua::Function<void(lua::Reference&&)>::Wrap<&Test::F3>, lua::STATIC_FUNCTION },
        { "F4", lua::Function<void(lua::WeakReference)>::Wrap<&Test::F4>, lua::STATIC_FUNCTION },
        { "F5", lua::Function<void(const lua::WeakReference&)>::Wrap<&Test::F5>, lua::STATIC_FUNCTION },
        { "F6", lua::Function<void(lua::WeakReference&&)>::Wrap<&Test::F6>, lua::STATIC_FUNCTION },
        { "F7", lua::Function<void(lua::TypedReference<int>)>::Wrap<&Test::F7>, lua::STATIC_FUNCTION },
        { "F8", lua::Function<void(const lua::TypedReference<int>&)>::Wrap<&Test::F8>, lua::STATIC_FUNCTION },
        { "F9", lua::Function<void(lua::TypedReference<int>&&)>::Wrap<&Test::F9>, lua::STATIC_FUNCTION },
        { "F10", lua::Function<void(lua::Reference)>::Wrap<&Test::F10>, lua::STATIC_FUNCTION },
        { "F11", lua::Function<void(const lua::Reference&)>::Wrap<&Test::F11>, lua::STATIC_FUNCTION },
        { "F12", lua::Function<void(lua::Reference&&)>::Wrap<&Test::F12>, lua::STATIC_FUNCTION },
        { "F13", lua::Function<void(lua::WeakReference)>::Wrap<&Test::F13>, lua::STATIC_FUNCTION },
        { "F14", lua::Function<void(const lua::WeakReference&)>::Wrap<&Test::F14>, lua::STATIC_FUNCTION },
        { "F15", lua::Function<void(lua::WeakReference&&)>::Wrap<&Test::F15>, lua::STATIC_FUNCTION },
        { "F16", lua::Function<void(lua::TypedReference<Object*>)>::Wrap<&Test::F16>, lua::STATIC_FUNCTION },
        { "F17", lua::Function<void(const lua::TypedReference<Object*>&)>::Wrap<&Test::F17>, lua::STATIC_FUNCTION },
        { "F18", lua::Function<void(lua::TypedReference<Object*>&&)>::Wrap<&Test::F18>, lua::STATIC_FUNCTION },
        { "F19", lua::Function<lua::Reference(lua_State*)>::Wrap<&Test::F19>, lua::STATIC_FUNCTION },
        { "F20", lua::Function<void(lua::TypedReference<Object*>)>::Wrap<&Test::F20>, lua::STATIC_FUNCTION },
};

void runtest (void)
{
    lua::State L;
    L.loadlib (luaopen_base, "");

    lua::register_class<Object> (L, "Object");
    lua::register_class<Test> (L, "Test");

    runlua (L, "function check (value, message) assert (value, message) print (message..': passed') end");

    runlua (L, "Test.F1 (201) Test.F2 (202) Test.F3 (203) Test.F4 (204) Test.F5 (205)"
    "Test.F6 (206) Test.F7 (207) Test.F8 (208) Test.F9 (209)");
    dontrunlua (L, "Test.F7 (nil)");
    dontrunlua (L, "Test.F7 ({})");
    dontrunlua (L, "Test.F7 (true)");
    runlua (L, "Test.F10 (Object (210)) Test.F11 (Object (211)) Test.F12 (Object (212))"
            "Test.F13 (Object (213)) Test.F14 (Object (214)) Test.F15 (Object (215))"
            "Test.F16 (Object (216)) Test.F17 (Object (217)) Test.F18 (Object (218))"
            "check (Test.F19 () == 219, 'return integer reference by value')");
    runlua (L, "Test.F20 (nil)");
}
