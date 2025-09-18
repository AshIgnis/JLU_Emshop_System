cmake_minimum_required(VERSION 3.5) # CMake install : https://cmake.org/download/
project(emshop_tcp_client LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "D:/Qt/6.9.1/mingw_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用Qt5，移除WebSockets依赖，使用Network进行TCP连接
find_package(Qt5 COMPONENTS Widgets Network REQUIRED) # Qt COMPONENTS

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 使用TCP客户端源文件
set(SOURCES
    main.cpp
    MainWindow.cpp
    LoginDialogTcp.cpp
    EmshopTcpClient.cpp
    ClientAdapter.cpp
    ProductListWidget.cpp
    CartWidget.cpp
    OrderWidget.cpp
)

set(HEADERS
    MainWindow.h
    LoginDialog.h
    EmshopTcpClient.h
    ClientAdapter.h
    ProductListWidget.h
    CartWidget.h
    OrderWidget.h
)

add_executable(${PROJECT_NAME}
    WIN32 # Windows GUI应用程序
    ${SOURCES}
    ${HEADERS}
) 

# 链接Qt5库，移除WebSockets依赖
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Network)

# 设置输出名称
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "emshop_tcp_client"
)

message(STATUS "====================================")
message(STATUS "JLU电商系统 TCP客户端配置")
message(STATUS "  - 兼容服务器: EmshopNettyServer.java")
message(STATUS "  - 通信协议: TCP (与Netty DelimiterBasedFrameDecoder兼容)")
message(STATUS "  - Qt版本: ${Qt5_VERSION}")
message(STATUS "  - 输出名称: emshop_tcp_client.exe")
message(STATUS "====================================")