cmake_minimum_required(VERSION 3.10)
project(code)
add_compile_options(-g -Wall)
add_subdirectory(../googletest googletest)
add_subdirectory(../tinyxml2 tinyxml2)
add_executable(sends sends.c)
add_executable(check check.cc)
add_executable(xml xml.cc)
target_link_libraries(check gmock gmock_main)
target_link_libraries(xml tinyxml2)
