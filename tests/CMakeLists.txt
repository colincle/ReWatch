find_package(Qt6 REQUIRED COMPONENTS Test)
enable_testing()

file(GLOB_RECURSE TEST_SOURCES core/*.cpp ui/*.cpp utils/*.cpp)

add_executable(ReWatchTests
    main.cpp
    ${TEST_SOURCES}
    $<TARGET_OBJECTS:ReWatchLib>
)

target_include_directories(ReWatchTests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/tinyfiledialogs
)

file(GLOB_RECURSE TEST_INCLUDE_DIRS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/include/*)
list(APPEND TEST_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)
foreach(DIR ${TEST_INCLUDE_DIRS})
    if(IS_DIRECTORY ${DIR})
        target_include_directories(ReWatchTests PRIVATE ${DIR})
    endif()
endforeach()

target_link_libraries(ReWatchTests
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    Qt6::Svg
    Qt6::Multimedia
    Qt6::Test
)

add_test(NAME ReWatchTests COMMAND ReWatchTests)
