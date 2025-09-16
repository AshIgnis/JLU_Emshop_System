cmake_minimum_required(VERSION 3.5) # CMake install : https://cmake.org/download/
project(qt_client LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "D:/Qt/6.9.1/mingw_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt5 COMPONENTS Widgets WebSockets Network REQUIRED) # Qt COMPONENTS

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Collect all source files
set(SOURCES
    main.cpp
    MainWindow.cpp
    LoginDialog.cpp
    EmshopClient.cpp
    ProductListWidget.cpp
    CartWidget.cpp
    OrderWidget.cpp
)

set(HEADERS
    MainWindow.h
    LoginDialog.h
    EmshopClient.h
    ProductListWidget.h
    CartWidget.h
    OrderWidget.h
)

add_executable(${PROJECT_NAME}
    WIN32 # If you need a terminal for debug, please comment this statement 
    ${SOURCES}
    ${HEADERS}
) 
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::WebSockets Qt5::Network) # Qt5 Shared Library