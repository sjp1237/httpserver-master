# 包含头文件的路径 
project(sample1)
cmake_minimum_required(VERSION 3.18)
#set(CMAKE_CXX_FLAGES  -std=c++11)
add_compile_options(-std=c++11 -Wall )
set(CMAKE_BUILD_TYPE  Debug)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
# 设置库文件的生成路径
set(LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(SRC_LIST sample_request_unittest.cc ../httpconn.cc ../../tool/tool.cc)
# 链接依赖的库: gtest库,pthread库 
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(sample1 gtest_main)
target_link_libraries(sample1 pthread)
target_link_libraries(sample1  gtest) 


