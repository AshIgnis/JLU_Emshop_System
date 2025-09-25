cmake_minimum_required(VERSION 3.16)
project(qtclient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6。
# 推荐设置环境变量 Qt6_DIR 指向你的Qt安装目录, 例如:
# $env:Qt6_DIR="D:\Qt\6.9.1\mingw_64"
# 如果CMake找不到Qt，请确保该环境变量已设置。
find_package(Qt6 COMPONENTS Widgets Network REQUIRED)

# 手动指定源文件路径
set(SOURCES
    qt_client/src/main.cpp
    qt_client/src/MainWindow.cpp
    qt_client/src/LoginDialog.cpp
    qt_client/src/EmshopTcpClient.cpp
    qt_client/src/ClientAdapter.cpp
    qt_client/src/ProductListWidget.cpp
    qt_client/src/CartWidget.cpp
    qt_client/src/OrderWidget.cpp
)

set(HEADERS
    qt_client/src/MainWindow.h
    qt_client/src/LoginDialog.h
    qt_client/src/EmshopTcpClient.h
    qt_client/src/ClientAdapter.h
    qt_client/src/ProductListWidget.h
    qt_client/src/CartWidget.h
    qt_client/src/OrderWidget.h
)

add_executable(${PROJECT_NAME}
    WIN32 # GUI应用程序
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network)

# 设置输出名称和路径
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "EmshopQtClient"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)