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

# 声明一个可执行文件，并包含main.cpp、decisionmaker.cpp和global_vars.cpp
add_executable(main 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/decision_maker_base.cpp
    ${CMAKE_SOURCE_DIR}/src/decision_maker_robot.cpp
    ${CMAKE_SOURCE_DIR}/src/decision_maker_ship.cpp
    ${CMAKE_SOURCE_DIR}/src/decision_maker_robot_jamControl.cpp
    ${CMAKE_SOURCE_DIR}/src/decision_maker_boat_jamControl.cpp
    ${CMAKE_SOURCE_DIR}/src/global_vars.cpp
)
