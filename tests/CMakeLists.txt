add_executable(test_queue test_queue.cc)
target_link_libraries(test_queue PUBLIC gtest thread-safe::collections)

add_test(test_queue test_queue)
