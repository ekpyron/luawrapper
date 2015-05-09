#include "common.h"

enum ConstructionType {
    DEFAULT, COPY, MOVE, EXPLICIT
};

class Base {
public:
    virtual bool checkstate (ConstructionType constructiontype, bool copied, bool moved, int expected_value) const {
        return false;
    }
    static lua::functionlist lua_functions;
};

lua::functionlist Base::lua_functions = {
};

class Object : public Base
{
public:
    Object (void) : construction (DEFAULT), destructed (false), copy_assigned (false), move_assigned (false), value (0) {
        count++;
    }
    Object (int v) : construction (EXPLICIT), destructed (false), copy_assigned (false), move_assigned (false), value (v) {
        count++;
    }
    Object (const Object &o) : construction (COPY), destructed (false), copy_assigned (false), move_assigned (false),
                               value (o.value) {
        count++;
    }
    Object (Object &&o) : construction (MOVE), destructed (false), copy_assigned (false), move_assigned (false),
                          value (o.value) {
        count++;
    }
    ~Object (void) {
        destructed = true;
    }
    Object &operator= (const Object &o) {
        copy_assigned = true;
        value = o.value;
        return *this;
    }
    Object &operator= (Object &&o) {
        move_assigned = true;
        value = o.value;
        o.value = -1;
        return *this;
    }

    const int &GetValue (void) const {
        return value;
    }

    int value;

    ConstructionType construction;
    bool destructed;
    bool copy_assigned;
    bool move_assigned;

    virtual bool checkstate (ConstructionType constructiontype, bool copied, bool moved, int expected_value) const {
        return construction == constructiontype && copy_assigned == copied && move_assigned == moved && !destructed
               && value == expected_value;
    }

    static int count;
    static lua::functionlist lua_functions;
};

int Object::count = 0;

lua::functionlist Object::lua_functions = {
        { lua::Overload<lua::Constructor<Object>::Wrap, lua::Constructor<Object, int>::Wrap>, lua::CONSTRUCTOR },
        { lua::Destructor<Object>::Wrap, lua::DESTRUCTOR },
        lua::BaseClass<Base>,
        { "GetValue", lua::Function<const int&(void)const>::Wrap<Object, &Object::GetValue> }
};
