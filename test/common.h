#ifndef COMMON_H
#define COMMON_H

#include <luawrapper/luawrapper.h>
#include <iostream>
#include <vector>
#include <cstdlib>

static bool verbose = true;

class RequireOnce
{
public:
    RequireOnce (const char *name) {
        id = flags.size ();
        flags.push_back (false);
        names.push_back (name);
    }
    void operator () (void) {
        if (flags[id]) {
            std::cerr << "ERROR: " << names[id] << " run multiple times" << std::endl;
            std::exit (EXIT_FAILURE);
        }
        if (verbose) {
            std::cout << names[id] << " run" << std::endl;
        }
        flags[id] = true;
    }
    void reset (void) {
        verify ();
        flags[id] = false;
    }
    void verify (void) {
        verify (id);
    }
    static void verify_all (void) {
        for (int id = 0; id < flags.size (); id++) {
            verify (id);
        }
    }
    static void verify_and_reset_all (void) {
        for (int id = 0; id < flags.size (); id++) {
            verify (id);
            flags[id] = false;
        }
    }
private:
    static void verify (int id) {
        if (!flags[id]) {
            std::cerr << "ERROR: " << names[id] << " was not run" << std::endl;
            std::exit (EXIT_FAILURE);
        }
    }
    int id;
    static std::vector<std::string> names;
    static std::vector<bool> flags;
};

std::vector<bool> RequireOnce::flags;
std::vector<std::string> RequireOnce::names;

void runlua (lua_State *L, const char *code) {
    if (luaL_dostring (L, code)) {
        std::string msg;
        if (!lua_isnil (L, -1)) {
            size_t len = 0;
            auto str = lua_tolstring (L, -1, &len);
            msg = std::string (str, len);
        }
        std::cerr << "LUA ERROR: " << msg << std::endl;
        std::exit (EXIT_FAILURE);
    }
}

void dontrunlua (lua_State *L, const char *code) {
    int top = lua_gettop (L);
    if (!luaL_dostring (L, code)) {
        std::cerr << "no lua error although expected in: " << code << std::endl;
        std::exit (EXIT_FAILURE);
    }
    lua_settop (L, top);
}

void check (bool condition, const std::string &msg) {
    if (!condition) {
        std::cerr << "ERROR: " << msg << ": failed" << std::endl;
        std::exit (EXIT_FAILURE);
    } else if (verbose) {
        std::cout << msg << ": passed" << std::endl;
    }
}

void runtest (void);

int main (int argc, char *argv[])
{
    try {
        runtest ();
        RequireOnce::verify_all ();
    } catch (std::exception &e) {
        std::cerr << "EXCEPTION: " << e.what () << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#endif /* !defined COMMON_H */
