cmake_minimum_required(VERSION 3.16)
project(ReWatch)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Concurrent Svg Multimedia)

file(GLOB_RECURSE ALL_SOURCES
    src/*.cpp
    include/**/*.hpp
    external/tinyfiledialogs/*.cpp
)

# Separate main.cpp so the object library doesn't define main()
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

file(GLOB_RECURSE INCLUDE_DIRS LIST_DIRECTORIES true include/*)
list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

foreach(DIR ${INCLUDE_DIRS})
    if(IS_DIRECTORY ${DIR})
        include_directories(${DIR})
    endif()
endforeach()

# Third-party headers live under external/ (each library in its own folder)
file(GLOB EXTERNAL_DIRS LIST_DIRECTORIES true external/*)
list(APPEND EXTERNAL_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external)
foreach(DIR ${EXTERNAL_DIRS})
    if(IS_DIRECTORY ${DIR})
        include_directories(${DIR})
    endif()
endforeach()

set(QT_LIBS
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    Qt6::Svg
    Qt6::Multimedia
)

# Object library — compiled once, shared by the app and the test suite
add_library(ReWatchLib OBJECT ${ALL_SOURCES})
target_include_directories(ReWatchLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${INCLUDE_DIRS}
)
target_link_libraries(ReWatchLib ${QT_LIBS})

option(SHIPPED "Build for app bundle deployment" OFF)
option(BUILD_TESTS "Build test suite" OFF)

add_executable(ReWatch src/main.cpp $<TARGET_OBJECTS:ReWatchLib>)
target_include_directories(ReWatch PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${INCLUDE_DIRS}
)
target_link_libraries(ReWatch ${QT_LIBS})

if(SHIPPED)
    target_compile_definitions(ReWatch PRIVATE SHIPPED)
endif()

# EXCLUDE_FROM_ALL: tests are never built by the default "all" target (i.e. a plain
# `cmake --build build`). Build them explicitly with `--target ReWatchTests`.
if(BUILD_TESTS)
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()
