cmake_minimum_required(VERSION 3.0.0)
project(luawrapper)

add_subdirectory (luawrapper)

include (CTest)

if (BUILD_TESTING)
add_subdirectory (test)
endif (BUILD_TESTING)
