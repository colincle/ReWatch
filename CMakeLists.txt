cmake_minimum_required(VERSION 3.16)
project(MovieTracker)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Concurrent Svg)

include_directories(
	include
	include/core
	include/ui
	external/json
	external/tinyfiledialogs
)

file(GLOB_RECURSE SOURCES
	src/*.cpp
	include/**/*.hpp
	external/tinyfiledialogs/*.cpp
)

add_executable(MovieTracker ${SOURCES})

target_include_directories(MovieTracker PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/core
	${CMAKE_CURRENT_SOURCE_DIR}/include/ui
)

target_link_libraries(MovieTracker
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	Qt6::Network
	Qt6::Concurrent
	Qt6::Svg
)
