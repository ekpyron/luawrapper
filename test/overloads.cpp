#include "Object.h"

class Test
{
public:
    int F (void) {
        return 400;
    }
    int F (int value) {
        return 400 + value;
    }
    int F (std::string str) {
        return 400 + str.length ();
    }
    int F (Object *object) {
        return 500;
    }
    int F (int value, Object *object) {
        return value + 501;
    }
    int F (Object *object, int value) {
        return value + 502;
    }

    static int F2 (void) {
        return 600;
    }
    static int F2 (int value) {
        return 601 + value;
    }
    static int F3 (std::string value) {
        return 602 + value.length ();
    }

    static lua::functionlist lua_functions;
};

lua::functionlist Test::lua_functions = {
        { lua::Constructor<Test>::Wrap, lua::CONSTRUCTOR },
        { "F", lua::Overload<lua::Function<int(void)>::Wrap<Test, &Test::F>,
                lua::Function<int(int)>::Wrap<Test, &Test::F>,
                lua::Function<int(std::string)>::Wrap<Test, &Test::F>,
                lua::Function<int(Object*)>::Wrap<Test, &Test::F>,
                lua::Function<int(int,Object*)>::Wrap<Test, &Test::F>,
                lua::Function<int(Object*,int)>::Wrap<Test, &Test::F>> },
        { "F2", lua::Overload<lua::Function<int(void)>::Wrap<&Test::F2>,
                lua::Function<int(int)>::Wrap<&Test::F2>,
                lua::Function<int(std::string)>::Wrap<&Test::F3>>, lua::STATIC_FUNCTION}
};

void runtest (void)
{
    lua::State L;

    L.loadlib (luaopen_base, "");
    lua::register_class<Test> (L, "Test");
    lua::register_class<Object> (L, "Object");

    runlua (L, R"code(

function check (value, message)
  assert (value, message)
  print (message..": passed")
end

test = Test()
check (test.F() == 400, "overload int(void)")
check (test.F(42) == 442, "overload int(int)")
check (test.F("XYZ") == 403, "overload int(std::string)")
obj = Object()
check (test.F(obj) == 500, "overload int(Object)")
check (test.F(10,obj) == 511, "overload int(int, Object)")
check (test.F(obj,20) == 522, "overload int(Object, int)")
check (Test.F2() == 600, "static overload int(void)")
check (Test.F2(20) == 621, "static overload int(int)")
check (Test.F2("XYZ") == 605, "static overload int(std::string)")

)code");

}
