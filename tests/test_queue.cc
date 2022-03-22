#include <gtest/gtest.h>
#include <thread_safe/collections/collections.h>


TEST(test_queue, test_queue)
{
    thread_safe::Queue<int> queue{100};

    ASSERT_TRUE(queue.is_empty());
    ASSERT_FALSE(queue.is_full());

    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(queue.size(), i);
        queue.enqueue(i);
    }

    ASSERT_TRUE(queue.is_full());
    ASSERT_FALSE(queue.is_empty());

    for (int i = 0; i < 100; i++) {
        auto out = queue.dequeue();
        ASSERT_EQ(*out, i);
    }

    ASSERT_TRUE(queue.is_empty());
    ASSERT_FALSE(queue.is_full());
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

