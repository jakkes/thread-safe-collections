if (NOT TARGET gtest AND THREAD_SAFE_COLLECTIONS_BUILD_TESTS)
    add_subdirectory(googletest)
endif()
