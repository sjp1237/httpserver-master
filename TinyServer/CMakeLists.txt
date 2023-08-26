cmake_minimum_required(VERSION 2.8.3) #指定cmake的最小版本是2.8.3版本
project(main) #定义工程名为hello

AUX_SOURCE_DIRECTORY(./api/jsoncpp JSON_LIST)
AUX_SOURCE_DIRECTORY(./api API)
AUX_SOURCE_DIRECTORY(./RedisPool REDIS_POOL)
AUX_SOURCE_DIRECTORY(./log LOG)
set(CMAKE_BUILD_TYPE  Debug)
set(SRC main.cc MysqlPool/DBPool.cpp reactor/reactor.cc httpconn/httpconn.cc lst_timer/lst_timer.cc tcpserver/TcpServer.cc tool/tool.cc api/comm.cc api/ApiRegister.cpp )  #设置SRC变量为main.cpp hello.cpp
add_compile_options(-std=c++11)

add_executable(${PROJECT_NAME} ${JSON_LIST} ${API} ${REDIS_POOL} ${LOG}  ${SRC}) #编译文

target_link_libraries(main pthread)
target_link_libraries(main mysqlclient)