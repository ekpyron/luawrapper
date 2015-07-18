find_package (Lua51 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

option (BUILD_SHARED "Build luawrapper as shared library" ON)

if (BUILD_SHARED)
set (SHARED_FLAG "SHARED")
endif (BUILD_SHARED)

add_library (luawrapper ${SHARED_FLAG} helper_functions.cpp Reference.cpp WeakReference.cpp State.cpp)

set_target_properties (luawrapper PROPERTIES VERSION 0.1 SOVERSION 0)

target_link_libraries (luawrapper ${LUA_LIBRARIES})
target_include_directories (luawrapper SYSTEM PUBLIC ${LUA_INCLUDE_DIR})
target_include_directories (luawrapper INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..> $<INSTALL_INTERFACE:include/>)

install (TARGETS luawrapper DESTINATION lib EXPORT luawrapper)
install (EXPORT luawrapper DESTINATION lib/cmake/luawrapper)
file (GLOB DETAIL_HEADERS detail/*.h)
install (FILES luawrapper.h DESTINATION include/luawrapper)
install (FILES ${DETAIL_HEADERS} DESTINATION include/luawrapper/detail)
