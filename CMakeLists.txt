cmake_minimum_required(VERSION 2.8.4)
project(luawrapper)

find_package (Lua51 REQUIRED)

include_directories (${LUA_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp luawrapper.cpp)

add_executable(luawrapper ${SOURCE_FILES})

target_link_libraries (luawrapper ${LUA_LIBRARIES})
