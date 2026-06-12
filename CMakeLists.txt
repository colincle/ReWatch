cmake_minimum_required(VERSION 3.16)
project(MovieTracker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Concurrent Svg)

file(GLOB_RECURSE SOURCES
    src/*.cpp
    include/**/*.hpp
    external/tinyfiledialogs/*.cpp
)

file(GLOB_RECURSE INCLUDE_DIRS LIST_DIRECTORIES true include/*)
list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

foreach(DIR ${INCLUDE_DIRS})
    if(IS_DIRECTORY ${DIR})
        include_directories(${DIR})
    endif()
endforeach()

add_executable(MovieTracker ${SOURCES})

target_include_directories(MovieTracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${INCLUDE_DIRS}
)

target_link_libraries(MovieTracker
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    Qt6::Svg
)