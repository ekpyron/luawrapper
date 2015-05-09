set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable (baseclass baseclass.cpp)
target_link_libraries (baseclass luawrapper)
add_test (baseclass baseclass)

add_executable (static_arguments static_arguments.cpp)
target_link_libraries (static_arguments luawrapper)
add_test (static_arguments static_arguments)

add_executable (member_arguments member_arguments.cpp)
target_link_libraries (member_arguments luawrapper)
add_test (member_arguments member_arguments)

add_executable (static_returnvalues static_returnvalues.cpp)
target_link_libraries (static_returnvalues luawrapper)
add_test (static_returnvalues static_returnvalues)

add_executable (member_returnvalues member_returnvalues.cpp)
target_link_libraries (member_returnvalues luawrapper)
add_test (member_returnvalues member_returnvalues)

add_executable (references references.cpp)
target_link_libraries (references luawrapper)
add_test (references references)

add_executable (overloads overloads.cpp)
target_link_libraries (overloads luawrapper)
add_test (overloads overloads)
