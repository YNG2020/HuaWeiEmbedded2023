cmake_minimum_required(VERSION 3.0)

#项目名
project(MyProject)

# 用于设置 C++ 编译器的编译选项，-w 选项是用来关闭编译器的警告信息的。
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

# if (CMAKE_BUILD_TYPE STREQUAL "Release")
#    Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
# endif()

# 添加头文件目录
include_directories(${CMAKE_SOURCE_DIR}/src)

# 声明一个可执行文件，并包含main.cpp、struct.cpp
add_executable(main 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/struct.cpp
    ${CMAKE_SOURCE_DIR}/src/global_var.cpp)
