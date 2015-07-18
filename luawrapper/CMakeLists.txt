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

set (INCLUDE_INSTALL_DIR "include/" CACHE PATH "Include install directory")
set (LIB_INSTALL_DIR "lib/" CACHE PATH "Library install directory")

include (CMakePackageConfigHelpers)
configure_package_config_file (luawrapper-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/luawrapper-config.cmake INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/luawrapper PATH_VARS INCLUDE_INSTALL_DIR)

install (TARGETS luawrapper DESTINATION ${LIB_INSTALL_DIR} EXPORT luawrapper)
file (GLOB DETAIL_HEADERS detail/*.h)
install (FILES luawrapper.h DESTINATION ${INCLUDE_INSTALL_DIR}/luawrapper)
install (FILES ${DETAIL_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/luawrapper/detail)
install (EXPORT luawrapper FILE luawrapper-target.cmake DESTINATION ${LIB_INSTALL_DIR}/cmake/luawrapper)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/luawrapper-config.cmake DESTINATION ${LIB_INSTALL_DIR}/cmake/luawrapper)
