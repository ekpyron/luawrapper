#include <iostream>
#include <sstream>
#include <typeinfo>
#include "luawrapper.h"

class BaseTest
{
public:
    BaseTest (void) {
    }
    virtual ~BaseTest (void) {
    }

    void BaseFunc (void) {
        std::cout << "BaseTest::BaseFunc ()" << std::endl;
    }
    static lua::functionlist lua_functions;
};

lua::functionlist BaseTest::lua_functions = {
        { "BaseFunc", lua::Function<void(void)>::Wrap<BaseTest, &BaseTest::BaseFunc> }
};

class Test;

class Test : public BaseTest
{
public:
    template<typename... Args>
    Test (Args... args) {
        Print (args...);
        test = sizeof... (args);
        destroyed = false;
    }

    Test (const Test &t) {
        std::cout << "Test::Test (const Test&)" << std::endl;
        test = t.test;
        destroyed = false;
    }

    Test (Test &&t) {
        std::cout << "Test::Test (Test&&)" << std::endl;
        test = t.test;
        destroyed = false;
    }

    Test &operator= (const Test &t) {
        std::cout << "Test &Test::operator= (const Test&)" << std::endl;
        test = t.test;
        return *this;
    }
    Test &operator= (Test &&t) {
        std::cout << "Test &Test::operator= (const Test&)" << std::endl;
        test = t.test;
        return *this;
    }

    template<typename T, typename...Args>
    void Print (T t, Args... args) const {
        std::cout << "ARG: " << t << " (" << typeid (T).name () << ")" << std::endl;
        Print (args...);
    }
    void Print(void) const {
    }

    ~Test (void) {
        std::cout << "Test::~Test (void) " << test << std::endl;
        if (destroyed) std::cout << "DOUBLEFREED" << std::endl;
        destroyed = true;
    }

    int fn (int i, float v) {
        std::cout << "Test::fn (" << i << ", " << v << ")" << std::endl;
        return 77;
    }

    void SetTest (const int &t) {
        std::cout << "SetTest " << t << std::endl;
        test = t;
    }
    int GetTest (void) {
        return test;
    }

    void Overloaded (const int &i) {
        std::cout << "Overloaded (const int &): " << i << std::endl;
    }

    void Overloaded (const std::string &str) {
        std::cout << "Overloaded (const std::string &): " << str << std::endl;
    }

    void Overloaded (const int &i, const std::string &str) {
        std::cout << "Overloaded (const int &, const std::string &): " << i << ", " << str << std::endl;
    }

    void Overloaded (const Test &test) {
        std::cout << "Overloaded (Test &): " << test.test << std::endl;
    }

    lua::ManualReturn GetValue (lua_State *L, const std::string &str) {
        lua::push<int> (L, str.length ());
        return lua::ManualReturn ();
    }

    void SetValue (const std::string &str, int v) {
        std::cout << str << " = " << v << std::endl;
    }

    void SetLuaObject (lua::Reference _ref) {
        std::cout << "Calling set test..." << std::endl;
        if (_ref.valid ()) {
            if (_ref.convert<Test> () == this) {
                std::cout << "Selfreference! Bad bad!" << std::endl;
                return;
            }
            ref = std::move (_ref);
            ref.convert<Test> ()->SetTest (80);
        }
    }

    lua::WeakReference GetLuaObject (void) {
        return ref;
    }

    static lua::functionlist lua_functions;

    lua::WeakReference ref;

    bool destroyed;

    int test;
};

lua::functionlist Test::lua_functions = {
        { "SetTest", lua::Function<void(const int&)>::Wrap<Test, &Test::SetTest> },
        { "GetTest", lua::Function<int(void)>::Wrap<Test, &Test::GetTest> },
        { "__tostring", [] (lua_State *L) -> int { lua_pushliteral (L, "[Test]"); return 1; }, lua::MetaFunction },
        { "StaticTest", [] (lua_State *L) -> int {std::cout << "StaticTest" << std::endl; return 0; }, lua::StaticFunction },
        { "Print", lua::Overload<lua::Function<void(const int&)>::Overload<Test, &Test::Overloaded>,
        lua::Function<void(const std::string&)>::Overload<Test, &Test::Overloaded>,
        lua::Function<void(const int&, const std::string&)>::Overload<Test, &Test::Overloaded>,
        lua::Function<void(const Test&)>::Overload<Test, &Test::Overloaded>>::Function },
        { "Print2", lua::Function<void(const int&, const std::string&)>::Wrap<Test, &Test::Overloaded> },
        { lua::Destruct<Test>, lua::Destructor },
        { lua::Overload<lua::Construct<Test>, lua::Construct<Test,int>, lua::Construct<Test,std::string>>::Construct, lua::Constructor },
        { "__call", lua::Function<void(const Test&)>::Wrap<Test, &Test::Overloaded, 1>, lua::MetaFunction },
        { lua::Function<lua::ManualReturn(lua_State *L, const std::string&)>::Wrap<Test, &Test::GetValue>, lua::IndexFunction },
        { lua::Function<void(const std::string&, int)>::Wrap<Test, &Test::SetValue, 1>, lua::NewIndexFunction },
        { "SetLuaObject", lua::Function<void(lua::Reference)>::Wrap<Test, &Test::SetLuaObject> },
        { "GetLuaObject", lua::Function<lua::WeakReference(void)>::Wrap<Test, &Test::GetLuaObject> },
        BaseTest::lua_functions
};

/*namespace lua {
template<>
struct Functions<Test> {
    static lua::functionlist lua_functions;
};
lua::functionlist Functions<Test>::lua_functions = {
        { "Test", [] (lua_State *L) -> int {std::cout << "Hm..." << std::endl; return 0; }}
};
}*/

int panicfn (lua_State *L)
{
    if (lua_isstring (L, -1))
    {
        std::cerr << "ERROR: " << lua_tostring (L, -1) << std::endl;
    }
    abort ();
}

int main (int argc, char *argv[])
{
    lua::State L;
    L.loadlib (luaopen_base, "");
    L.loadlib (luaopen_string, LUA_STRLIBNAME);
    L.loadlib (luaopen_table, LUA_TABLIBNAME);
    L.loadlib (luaopen_package, LUA_LOADLIBNAME);
    L.loadlib (luaopen_math, LUA_MATHLIBNAME);
    lua_atpanic (L, panicfn);

    lua::register_class<Test> (L, "Test");

    lua::push<Test> (L, 42);
    lua_setglobal (L, "test");

    std::string line;
    std::string inputbuffer;
    while (true)
    {
        std::getline (std::cin, line);
        if (!line.compare ("exit") || !line.compare ("quit"))
            break;


        std::string str = inputbuffer + line + '\n';
        int status = luaL_loadbuffer (L, str.data (), str.length (), "console input");
        if (status)
        {
            std::string err;
            if (!lua_isnil (L, -1))
            {
                const char *msg = lua_tostring (L, -1);
                if (msg) err = msg;
            }

            if (err.size () > 7 && !err.substr (err.size () - 6, 5).compare ("<eof>"))
            {
                inputbuffer.append (line + '\n');
            }
            else
            {
                std::cerr << err << std::endl;
                inputbuffer.clear ();
            }
        }
        else
        {
            if (lua_pcall (L, 0, LUA_MULTRET, 0))
            {
                if (!lua_isnil (L, -1))
                {
                    const char *msg = lua_tostring (L, -1);
                    if (msg) std::cerr << msg << std::endl;
                }
            }
            inputbuffer.clear ();
        }

        lua_gc (L, LUA_GCSTEP, 100);

        lua_settop (L, 0);
    }
}
