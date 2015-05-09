#include "common.h"

RequireOnce constructor ("constructor");
RequireOnce destructor ("destructor");
RequireOnce base_constructor ("base constructor");
RequireOnce base_destructor ("base destructor");


class Base
{
public:
    Base (void) : basevalue (66) {
        base_constructor ();
    }
    ~Base (void) {
        base_destructor ();
    }

    void SetBaseValue (int i) {
        basevalue = i;
    }

    int basevalue;
    static lua::functionlist lua_functions;
};

lua::functionlist Base::lua_functions = {
        { lua::Constructor<Base>::Wrap, lua::CONSTRUCTOR },
        { lua::Destructor<Base>::Wrap, lua::DESTRUCTOR },
        { "SetBaseValue", lua::Function<void(int)>::Wrap<Base, &Base::SetBaseValue> }

};


class Test : public Base
{
public:
    Test (void) : value (42) {
        constructor ();
    }
    ~Test (void) {
        destructor ();
    }

    void SetValue (int i) {
        value = i;
    }

    int value;

    static lua::functionlist lua_functions;
};

lua::functionlist Test::lua_functions = {
        { lua::Constructor<Test>::Wrap, lua::CONSTRUCTOR },
        { lua::Destructor<Test>::Wrap, lua::DESTRUCTOR },
        lua::BaseClass<Base>,
        { "SetValue", lua::Function<void(int)>::Wrap<Test, &Test::SetValue> }
};

void runtest (void)
{
    {
        lua::State L;

        lua::register_class<Base> (L, "Test");
        lua::register_class<Test> (L, "Test");

        runlua (L, "test = Test()");

        lua_getglobal (L, "test");
        Test *test = lua::pull<Test*> (L, -1);
        lua_pop (L, 1);

        check (test->value == 42, "correctness of initial member value");
        runlua (L, "test.SetValue (21)");
        check (test->value == 21, "correctness of member value after assignment");

        check (test->basevalue == 66, "correctness of initial base member value");
        runlua (L, "test.SetBaseValue (33)");
        check (test->basevalue == 33, "correctness of base member value after assignment");
    }
    RequireOnce::verify_and_reset_all ();
    {
        lua::State L;

        lua::register_class<Base> (L, "Test");
        lua::register_class<Test> (L, "Test");

        runlua (L, "test = Test()");

        lua_getglobal (L, "test");
        Test *test = lua::pull<Test*> (L, -1);
        lua_pop (L, 1);


        check (test->value == 42, "correctness of initial member value");
        runlua (L, "test.SetValue (21)");
        check (test->value == 21, "correctness of member value after assignment");

        check (test->basevalue == 66, "correctness of initial base member value");
        runlua (L, "test.SetBaseValue (33)");
        check (test->basevalue == 33, "correctness of base member value after assignment");

    }
}
