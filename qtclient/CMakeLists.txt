cmake_minimum_required(VERSION 3.16)
project(emshop_qtclient LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Allow users to point to a specific Qt installation via environment variable
if(DEFINED ENV{Qt6_DIR})
    list(APPEND CMAKE_PREFIX_PATH $ENV{Qt6_DIR})
endif()
if(DEFINED ENV{Qt5_DIR})
    list(APPEND CMAKE_PREFIX_PATH $ENV{Qt5_DIR})
endif()

set(QT_REQUIRED_COMPONENTS Widgets Network Gui Core)

find_package(Qt6 COMPONENTS ${QT_REQUIRED_COMPONENTS} QUIET)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBS Qt6::Widgets Qt6::Network Qt6::Gui Qt6::Core)
else()
    find_package(Qt5 COMPONENTS ${QT_REQUIRED_COMPONENTS} REQUIRED)
    set(QT_VERSION_MAJOR 5)
    set(QT_LIBS Qt5::Widgets Qt5::Network Qt5::Gui Qt5::Core)
endif()

add_subdirectory(src)
