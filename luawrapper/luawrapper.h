/*
 * C++ helper and wrapper functions for Lua.
 *
 * Copyright (c) 2015 Daniel Kirchner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef LUAWRAPPER_H
#define LUAWRAPPER_H

#include <vector>
#include <stdexcept>
#include <limits>
#include <iostream>
#include <initializer_list>
#include <typeinfo>
#include <type_traits>
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "detail/template_helpers.h"
#include "detail/helper_functions.h"
#include "detail/State.h"
#include "detail/Reference.h"
#include "detail/TypedReference.h"
#include "detail/WeakReference.h"
#include "detail/functions.h"
#include "detail/push.h"
#include "detail/Type.h"
#include "detail/ArgHandler.h"
#include "detail/CallHelper.h"
#include "detail/Function.h"
#include "detail/ConstructHelper.h"
#include "detail/Constructor.h"
#include "detail/Overload.h"
#include "detail/pull.h"
#include "detail/register.h"

#endif /* !defined LUAWRAPPER_H */
