#include "Object.h"

class Test
{
public:
    int F1 (void) {
        return 101;
    }
    const int &F2 (void) {
        static int i = 102;
        return i;
    }
    int &F3 (void) {
        static int i = 103;
        return i;
    }
    int &&F4 (void) {
        static int i = 104;
        return std::move (i);
    }
    std::string F5 (void) {
        return "F5";
    }
    std::string &F6 (void) {
        static std::string s ("F6");
        return s;
    }
    const std::string &F7 (void) {
        static std::string s ("F7");
        return s;
    }
    std::string &&F8 (void) {
        static std::string s ("F8");
        return std::move (s);
    }
    lua::ManualReturn F9 (lua_State *L) {
        lua_pushliteral (L, "F9");
        return lua::ManualReturn ();
    }

    Object F10 (void) {
        return Object (53);
    }
    const Object &F11 (void) {
        static Object obj (54);
        return obj;
    }
    Object &F12 (void) {
        static Object obj (55);
        return obj;
    }
    Object &&F13 (void) {
        static Object obj (56);
        return std::move (obj);
    }

    Object *F14 (void) {
        return new Object (57);
    }

    const Object *F15 (void) {
        return new Object (58);
    }

    static lua::functionlist lua_functions;
};

lua::functionlist Test::lua_functions = {
        { lua::Constructor<Test>::Wrap, lua::CONSTRUCTOR },
        { "F1", lua::Function<int(void)>::Wrap<Test, &Test::F1> },
        { "F2", lua::Function<const int&(void)>::Wrap<Test, &Test::F2> },
        { "F3", lua::Function<int&(void)>::Wrap<Test, &Test::F3> },
        { "F4", lua::Function<int&&(void)>::Wrap<Test, &Test::F4> },
        { "F5", lua::Function<std::string(void)>::Wrap<Test, &Test::F5> },
        { "F6", lua::Function<std::string&(void)>::Wrap<Test, &Test::F6> },
        { "F7", lua::Function<const std::string&(void)>::Wrap<Test, &Test::F7> },
        { "F8", lua::Function<std::string&&(void)>::Wrap<Test, &Test::F8> },
        { "F9", lua::Function<lua::ManualReturn(lua_State*L)>::Wrap<Test, &Test::F9, -1> },
        { "F10", lua::Function<Object(void)>::Wrap<Test, &Test::F10> },
        { "F11", lua::Function<const Object&(void)>::Wrap<Test, &Test::F11> },
        { "F12", lua::Function<Object&(void)>::Wrap<Test, &Test::F12> },
        { "F13", lua::Function<Object&&(void)>::Wrap<Test, &Test::F13> },
        { "F14", lua::Function<Object*(void)>::Wrap<Test, &Test::F14> },
        { "F15", lua::Function<const Object*(void)>::Wrap<Test, &Test::F15> }
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
    runlua (L, "test = Test() check (test.F1() == 101, 'return int by value')"
            "check (test.F2() == 102, 'return int by const reference')"
            "check (test.F3() == 103, 'return int by reference')"
            "check (test.F4() == 104, 'return int by rvalue reference')"
            "check (test.F5() == 'F5', 'return string by value')"
            "check (test.F6() == 'F6', 'return string by reference')"
            "check (test.F7() == 'F7', 'return string by const reference')"
            "check (test.F8() == 'F8', 'return string by rvalue reference')"
            "check (test.F9() == 'F9', 'return string by rvalue reference')");

    runlua (L, "obj = test.F10 ()");
    lua_getglobal (L, "obj");
    check (getobj (L)->checkstate (MOVE, false, false, 53) && Object::count == 2, "return object by value");

    Object::count = 0;
    runlua (L, "obj = test.F11 ()");
    check (getobj (L)->checkstate (COPY, false, false, 54) && Object::count == 2, "return object by const reference");

    Object::count = 0;
    runlua (L, "obj = test.F12 ()");
    check (getobj (L)->checkstate (COPY, false, false, 55) && Object::count == 2, "return object by reference");

    Object::count = 0;
    runlua (L, "obj = test.F13 ()");
    check (getobj (L)->checkstate (MOVE, false, false, 56) && Object::count == 2, "return object by rvalue reference");

    Object::count = 0;
    runlua (L, "obj = test.F14 ()");
    check (getobj (L)->checkstate (EXPLICIT, false, false, 57) && Object::count == 1, "return object by pointer");

    Object::count = 0;
    runlua (L, "obj = test.F15 ()");
    check (getobj (L)->checkstate (EXPLICIT, false, false, 58) && Object::count == 1, "return object by const pointer");
}
