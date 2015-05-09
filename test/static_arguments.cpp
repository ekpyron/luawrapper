#include "common.h"
#include "Object.h"

class Test
{
public:
    static void F1 (int i) {
        check (i == 42, "integer passed by value");
    }
    static void F2 (const int &i) {
        check (i == 43, "integer passed by const reference");
    }
    static void F3 (int &&i) {
        check (i == 44, "integer passed by rvalue reference");
    }

    static void F4 (Object o) {
        check (o.checkstate (COPY, false, false, 45) && Object::count == 2, "object passed by value");
    }

    static void F5 (const Object &o) {
        check (o.checkstate (DEFAULT, false, false, 46) && Object::count == 1, "object passed by const reference");
    }
    static void F6 (Object &o) {
        check (o.checkstate (DEFAULT, false, false, 47) && Object::count == 1, "object passed by reference");
        o.value = 99;
    }
    static void F7 (Object &&o) {
        check (o.checkstate (DEFAULT, false, false, 48) && Object::count == 1, "object passed by rvalue reference");
        o.value = 100;
    }
    static void F8 (Object *o) {
        check (o->checkstate (DEFAULT, false, false, 49) && Object::count == 1, "object passed by pointer");
        o->value = 101;
    }
    static void F9 (const Object *o) {
        check (o->checkstate (DEFAULT, false, false, 50) && Object::count == 1, "object passed by const pointer");
    }
    static void F10 (Object o1, const Object &o2, Object &o3, Object &&o4, Object *o5, const Object *o6) {
        check (o1.checkstate (COPY, false, false, 51)
                && o2.checkstate (DEFAULT, false, false, 51)
                && o3.checkstate (DEFAULT, false, false, 51)
                && o4.checkstate (DEFAULT, false, false, 51)
                && o5->checkstate (DEFAULT, false, false, 51)
                && o6->checkstate (DEFAULT, false, false, 51)
                && Object::count == 2, "objects passed multiple times");
        o3.value = 100;
        check (o2.value == 100 && o4.value == 100 && o5->value == 100 && o6->value == 100, "modification of object passed multiple times");
    }

    static void F11 (std::string s) {
        check (!s.compare ("string passed by value"), s);
    }
    static void F12 (const std::string &s) {
        check (!s.compare ("string passed by const reference"), s);
    }
    static void F13 (std::string &&s) {
        check (!s.compare ("string passed by rvalue reference"), s);
    }

    static void F14 (Base &b) {
        check (b.checkstate (DEFAULT, false, false, 52) && Object::count == 1, "base passed by reference");
    }
    static void F15 (Base *b) {
        check (b->checkstate (DEFAULT, false, false, 53) && Object::count == 1, "base passed by pointer");
    }

    static lua::functionlist lua_functions;
};

lua::functionlist Test::lua_functions = {
        { "F1", lua::Function<void(int)>::Wrap<&Test::F1>, lua::STATIC_FUNCTION },
        { "F2", lua::Function<void(const int&)>::Wrap<&Test::F2>, lua::STATIC_FUNCTION },
        { "F3", lua::Function<void(int&&)>::Wrap<&Test::F3>, lua::STATIC_FUNCTION },
        { "F4", lua::Function<void(Object)>::Wrap<&Test::F4>, lua::STATIC_FUNCTION },
        { "F5", lua::Function<void(const Object&)>::Wrap<&Test::F5>, lua::STATIC_FUNCTION },
        { "F6", lua::Function<void(Object&)>::Wrap<&Test::F6>, lua::STATIC_FUNCTION },
        { "F7", lua::Function<void(Object&&)>::Wrap<&Test::F7>, lua::STATIC_FUNCTION },
        { "F8", lua::Function<void(Object*)>::Wrap<&Test::F8>, lua::STATIC_FUNCTION },
        { "F9", lua::Function<void(const Object*)>::Wrap<&Test::F9>, lua::STATIC_FUNCTION },
        { "F10", lua::Function<void(Object, const Object&, Object&, Object&&, Object*, const Object*)>::Wrap<&Test::F10>, lua::STATIC_FUNCTION },
        { "F11", lua::Function<void(std::string)>::Wrap<&Test::F11>, lua::STATIC_FUNCTION },
        { "F12", lua::Function<void(const std::string&)>::Wrap<&Test::F12>, lua::STATIC_FUNCTION },
        { "F13", lua::Function<void(std::string&&)>::Wrap<&Test::F13>, lua::STATIC_FUNCTION },
        { "F14", lua::Function<void(Base&)>::Wrap<&Test::F14>, lua::STATIC_FUNCTION },
        { "F15", lua::Function<void(Base*)>::Wrap<&Test::F15>, lua::STATIC_FUNCTION }
};

void runtest (void)
{
    lua::State L;
    lua::register_class<Object> (L, "Object");
    lua::register_class<Test> (L, "Test");
    runlua (L, "Test.F1(42) Test.F2(43) Test.F3(44)");

    Object *obj = lua::push (L, new Object ());
    obj->value = 45;
    lua_setglobal (L, "object");

    runlua (L, "Test.F4 (object)");

    obj->value = 46;
    Object::count = 1;
    runlua (L, "Test.F5 (object)");

    obj->value = 47;
    Object::count = 1;
    runlua (L, "Test.F6 (object)");
    check (obj->value == 99, "modification of object passed by reference");

    obj->value = 48;
    Object::count = 1;
    runlua (L, "Test.F7 (object)");
    check (obj->value == 100, "modification of object passed by rvalue reference");

    obj->value = 49;
    Object::count = 1;
    runlua (L, "Test.F8 (object)");
    check (obj->value == 101, "modification of object passed by pointer");

    obj->value = 50;
    Object::count = 1;
    runlua (L, "Test.F9 (object)");

    obj->value = 51;
    Object::count = 1;
    runlua (L, "Test.F10 (object, object, object, object, object, object)");

    runlua (L, "Test.F11 (\"string passed by value\")"
    "Test.F12 (\"string passed by const reference\")"
    "Test.F13 (\"string passed by rvalue reference\")");

    obj->value = 52;
    Object::count = 1;
    runlua (L, "Test.F14 (object)");

    obj->value = 53;
    Object::count = 1;
    runlua (L, "Test.F15 (object)");
}
