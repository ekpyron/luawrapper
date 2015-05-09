#include "Object.h"

class Test
{
public:
    static int F1 (void) {
        return 101;
    }
    static const int &F2 (void) {
        static int i = 102;
        return i;
    }
    static int &F3 (void) {
        static int i = 103;
        return i;
    }
    static int &&F4 (void) {
        static int i = 104;
        return std::move (i);
    }
    static std::string F5 (void) {
        return "F5";
    }
    static std::string &F6 (void) {
        static std::string s ("F6");
        return s;
    }
    static const std::string &F7 (void) {
        static std::string s ("F7");
        return s;
    }
    static std::string &&F8 (void) {
        static std::string s ("F8");
        return std::move (s);
    }
    static lua::ManualReturn F9 (lua_State *L) {
        lua_pushliteral (L, "F9");
        return lua::ManualReturn ();
    }

    static Object F10 (void) {
        return Object (53);
    }
    static const Object &F11 (void) {
        static Object obj (54);
        return obj;
    }
    static Object &F12 (void) {
        static Object obj (55);
        return obj;
    }
    static Object &&F13 (void) {
        static Object obj (56);
        return std::move (obj);
    }

    static Object *F14 (void) {
        return new Object (57);
    }

    static const Object *F15 (void) {
        return new Object (58);
    }

    static lua::functionlist lua_functions;
};

lua::functionlist Test::lua_functions = {
        { "F1", lua::Function<int(void)>::Wrap<&Test::F1>, lua::STATIC_FUNCTION },
        { "F2", lua::Function<const int&(void)>::Wrap<&Test::F2>, lua::STATIC_FUNCTION },
        { "F3", lua::Function<int&(void)>::Wrap<&Test::F3>, lua::STATIC_FUNCTION },
        { "F4", lua::Function<int&&(void)>::Wrap<&Test::F4>, lua::STATIC_FUNCTION },
        { "F5", lua::Function<std::string(void)>::Wrap<&Test::F5>, lua::STATIC_FUNCTION },
        { "F6", lua::Function<std::string&(void)>::Wrap<&Test::F6>, lua::STATIC_FUNCTION },
        { "F7", lua::Function<const std::string&(void)>::Wrap<&Test::F7>, lua::STATIC_FUNCTION },
        { "F8", lua::Function<std::string&&(void)>::Wrap<&Test::F8>, lua::STATIC_FUNCTION },
        { "F9", lua::Function<lua::ManualReturn(lua_State*L)>::Wrap<&Test::F9, -1>, lua::STATIC_FUNCTION },
        { "F10", lua::Function<Object(void)>::Wrap<&Test::F10>, lua::STATIC_FUNCTION },
        { "F11", lua::Function<const Object&(void)>::Wrap<&Test::F11>, lua::STATIC_FUNCTION },
        { "F12", lua::Function<Object&(void)>::Wrap<&Test::F12>, lua::STATIC_FUNCTION },
        { "F13", lua::Function<Object&&(void)>::Wrap<&Test::F13>, lua::STATIC_FUNCTION },
        { "F14", lua::Function<Object*(void)>::Wrap<&Test::F14>, lua::STATIC_FUNCTION },
        { "F15", lua::Function<const Object*(void)>::Wrap<&Test::F15>, lua::STATIC_FUNCTION }
};

Object *getobj (lua_State *L) {
    lua_getglobal (L, "obj");
    auto obj = lua::pull<Object*> (L, -1);
    lua_pop (L, 1);
    return obj;
}

void runtest (void)
{
    lua::State L;

    L.loadlib (luaopen_base, "");

    lua::register_class<Object> (L, "Object");
    lua::register_class<Test> (L, "Test");

    runlua (L, "function check (value, message) assert (value, message) print (message..': passed') end");
    runlua (L, "check (Test.F1() == 101, 'return int by value')"
            "check (Test.F2() == 102, 'return int by const reference')"
            "check (Test.F3() == 103, 'return int by reference')"
            "check (Test.F4() == 104, 'return int by rvalue reference')"
            "check (Test.F5() == 'F5', 'return string by value')"
            "check (Test.F6() == 'F6', 'return string by reference')"
            "check (Test.F7() == 'F7', 'return string by const reference')"
            "check (Test.F8() == 'F8', 'return string by rvalue reference')"
            "check (Test.F9() == 'F9', 'return string by rvalue reference')");

    runlua (L, "obj = Test.F10 ()");
    lua_getglobal (L, "obj");
    check (getobj (L)->checkstate (MOVE, false, false, 53) && Object::count == 2, "return object by value");

    Object::count = 0;
    runlua (L, "obj = Test.F11 ()");
    check (getobj (L)->checkstate (COPY, false, false, 54) && Object::count == 2, "return object by const reference");

    Object::count = 0;
    runlua (L, "obj = Test.F12 ()");
    check (getobj (L)->checkstate (COPY, false, false, 55) && Object::count == 2, "return object by reference");

    Object::count = 0;
    runlua (L, "obj = Test.F13 ()");
    check (getobj (L)->checkstate (MOVE, false, false, 56) && Object::count == 2, "return object by rvalue reference");

    Object::count = 0;
    runlua (L, "obj = Test.F14 ()");
    check (getobj (L)->checkstate (EXPLICIT, false, false, 57) && Object::count == 1, "return object by pointer");

    Object::count = 0;
    runlua (L, "obj = Test.F15 ()");
    check (getobj (L)->checkstate (EXPLICIT, false, false, 58) && Object::count == 1, "return object by const pointer");
}
