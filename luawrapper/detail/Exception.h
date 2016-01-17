/*
 * Copyright 2016 Daniel Kirchner
 *
 * This file is part of luawrapper.
 *
 * luawrapper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * luawrapper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with luawrapper.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LUAWRAPPER_EXCEPTION_H
#define LUAWRAPPER_EXCEPTION_H

#include <exception>

namespace lua {

class Exception : public std::exception {
public:
    Exception (lua_State *L, int level, const std::string &msg_) {
        luaL_where (L, level);
        size_t len = 0;
        const char *str = lua_tolstring (L, -1, &len);
        msg = std::string (str, len) + msg_;
        lua_pop (L, 1);
    }
    virtual ~Exception (void) {
    }
    virtual const char *what (void) const noexcept {
        return msg.c_str ();
    }
private:
    std::string msg;
};

} /* namespace lua */

#endif /* !defined LUAWRAPPER_EXCEPTION_H */
