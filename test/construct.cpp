#include "common.h"

class SelfRefTest
{
public:
    SelfRefTest (lua::Reference ref) {
        selfref = ref;
        value = 0;
    }
    void Test (void) {
        value = 0x42424242;
        check (selfref.convert<SelfRefTest*> ()->value == 0x42424242, "correctness of selfreference");
    }
    int value;
    lua::Reference selfref;
    static lua::functionlist lua_functions;
};

lua::functionlist SelfRefTest::lua_functions = {
        { lua::ConstructorWithSelfReference<SelfRefTest>::Wrap, lua::CONSTRUCTOR },
        { "Test", lua::Function<void(void)>::Wrap<SelfRefTest, &SelfRefTest::Test> }
};

void runtest (void)
{
    {
        lua::State L;
        L.loadlib (luaopen_base, "");
        lua::register_class<SelfRefTest> (L, "SelfRefTest");
        runlua (L, "test = SelfRefTest() test.Test()");
    }
}
