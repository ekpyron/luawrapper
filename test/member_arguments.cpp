#include "common.h"
#include "Object.h"

class Test
{
public:
    void F1 (int i) {
        check (i == 42, "integer passed by value");
    }
    void F2 (const int &i) {
        check (i == 43, "integer passed by const reference");
    }
    void F3 (int &&i) {
        check (i == 44, "integer passed by rvalue reference");
    }

    void F4 (Object o) {
        check (o.checkstate (COPY, false, false, 45) && Object::count == 2, "object passed by value");
    }

    void F5 (const Object &o) {
        check (o.checkstate (DEFAULT, false, false, 46) && Object::count == 1, "object passed by const reference");
    }
    void F6 (Object &o) {
        check (o.checkstate (DEFAULT, false, false, 47) && Object::count == 1, "object passed by reference");
        o.value = 99;
    }
    void F7 (Object &&o) {
        check (o.checkstate (DEFAULT, false, false, 48) && Object::count == 1, "object passed by rvalue reference");
        o.value = 100;
    }
    void F8 (Object *o) {
        check (o->checkstate (DEFAULT, false, false, 49) && Object::count == 1, "object passed by pointer");
        o->value = 101;
    }
    void F9 (const Object *o) {
        check (o->checkstate (DEFAULT, false, false, 50) && Object::count == 1, "object passed by const pointer");
    }
    void F10 (Object o1, const Object &o2, Object &o3, Object &&o4, Object *o5, const Object *o6) {
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

    void F11 (std::string s) {
        check (!s.compare ("string passed by value"), s);
    }
    void F12 (const std::string &s) {
        check (!s.compare ("string passed by const reference"), s);
    }
    void F13 (std::string &&s) {
        check (!s.compare ("string passed by rvalue reference"), s);
    }

    void F14 (Base &b) {
        check (b.checkstate (DEFAULT, false, false, 52) && Object::count == 1, "base passed by reference");
    }
    void F15 (Base *b) {
        check (b->checkstate (DEFAULT, false, false, 53) && Object::count == 1, "base passed by pointer");
    }

    static lua::functionlist lua_functions;
};

lua::functionlist Test::lua_functions = {
        { lua::Constructor<Test>::Wrap, lua::CONSTRUCTOR },
        { "F1", lua::Function<void(int)>::Wrap<Test, &Test::F1> },
        { "F2", lua::Function<void(const int&)>::Wrap<Test, &Test::F2> },
        { "F3", lua::Function<void(int&&)>::Wrap<Test, &Test::F3> },
        { "F4", lua::Function<void(Object)>::Wrap<Test, &Test::F4> },
        { "F5", lua::Function<void(const Object&)>::Wrap<Test, &Test::F5> },
        { "F6", lua::Function<void(Object&)>::Wrap<Test, &Test::F6> },
        { "F7", lua::Function<void(Object&&)>::Wrap<Test, &Test::F7> },
        { "F8", lua::Function<void(Object*)>::Wrap<Test, &Test::F8> },
        { "F9", lua::Function<void(const Object*)>::Wrap<Test, &Test::F9> },
        { "F10", lua::Function<void(Object, const Object&, Object&, Object&&, Object*, const Object*)>::Wrap<Test, &Test::F10> },
        { "F11", lua::Function<void(std::string)>::Wrap<Test, &Test::F11> },
        { "F12", lua::Function<void(const std::string&)>::Wrap<Test, &Test::F12> },
        { "F13", lua::Function<void(std::string&&)>::Wrap<Test, &Test::F13> },
        { "F14", lua::Function<void(Base&)>::Wrap<Test, &Test::F14> },
        { "F15", lua::Function<void(Base*)>::Wrap<Test, &Test::F15> }
};

void runtest (void)
{
    lua::State L;
    lua::register_class<Object> (L, "Object");
    lua::register_class<Test> (L, "Test");
    runlua (L, "test = Test () test.F1(42) test.F2(43) test.F3(44)");

    Object *obj = lua::push (L, new Object ());
    obj->value = 45;
    lua_setglobal (L, "object");

    runlua (L, "test.F4 (object)");

    obj->value = 46;
    Object::count = 1;
    runlua (L, "test.F5 (object)");

    obj->value = 47;
    Object::count = 1;
    runlua (L, "test.F6 (object)");
    check (obj->value == 99, "modification of object passed by reference");

    obj->value = 48;
    Object::count = 1;
    runlua (L, "test.F7 (object)");
    check (obj->value == 100, "modification of object passed by rvalue reference");

    obj->value = 49;
    Object::count = 1;
    runlua (L, "test.F8 (object)");
    check (obj->value == 101, "modification of object passed by pointer");

    obj->value = 50;
    Object::count = 1;
    runlua (L, "test.F9 (object)");

    obj->value = 51;
    Object::count = 1;
    runlua (L, "test.F10 (object, object, object, object, object, object)");

    runlua (L, "test.F11 (\"string passed by value\")"
    "test.F12 (\"string passed by const reference\")"
    "test.F13 (\"string passed by rvalue reference\")");

    obj->value = 52;
    Object::count = 1;
    runlua (L, "test.F14 (object)");

    obj->value = 53;
    Object::count = 1;
    runlua (L, "test.F15 (object)");
}
