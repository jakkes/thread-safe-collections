if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(MAIN_FILE ON)
else()
    set(MAIN_FILE OFF)
endif()


project(thread_safe_collections)
cmake_minimum_required(VERSION 3.18)

option(THREAD_SAFE_COLLECTIONS_BUILD_TESTS "Build tests" ${MAIN_FILE})

add_library(thread-safe-collections INTERFACE)
add_library(thread-safe::collections ALIAS thread-safe-collections)

target_include_directories(thread-safe-collections INTERFACE include/)

add_subdirectory(libs)

if(THREAD_SAFE_COLLECTIONS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
