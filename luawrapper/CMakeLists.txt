find_package (Lua51 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library (luawrapper helper_functions.cpp Reference.cpp WeakReference.cpp State.cpp)

target_link_libraries (luawrapper ${LUA_LIBRARIES})
target_include_directories (luawrapper SYSTEM PUBLIC ${LUA_INCLUDE_DIR})
target_include_directories (luawrapper INTERFACE ..)
