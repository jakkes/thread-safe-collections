#include <thread>
#include <atomic>

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
        ASSERT_EQ(queue.size(), 100 - i);
        
        auto out = queue.dequeue();
        ASSERT_EQ(*out, i);
    }

    ASSERT_TRUE(queue.is_empty());
    ASSERT_FALSE(queue.is_full());
}


TEST(test_queue, test_multiple_producer_consumer)
{
    thread_safe::Queue<size_t> queue{10};
    std::atomic<bool> running{true};
    std::atomic<size_t> out_sum{0};

    auto producer_fn = [&] () {
        for (size_t i = 0; i < 1000; i++) {
            auto queued{false};
            while (!queued) {
                queued = queue.enqueue(i, std::chrono::milliseconds(100));
            }
        }
    };

    auto consumer_fn = [&] () {
        size_t sum{0};
        while (running) {
            auto out = queue.dequeue(std::chrono::milliseconds(100));
            if (!out) continue;
            sum += *out;
        }
        out_sum += sum;
    };

    std::vector<std::thread> producers{};
    std::vector<std::thread> consumers{};

    for (int i = 0; i < 10; i++) producers.push_back(std::thread(producer_fn));
    for (int i = 0; i < 5; i++) consumers.push_back(std::thread(consumer_fn));

    for (auto &producer : producers) {
        producer.join();
    }

    while (!queue.is_empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    running = false;

    for (auto &consumer : consumers) {
        consumer.join();
    }

    ASSERT_EQ(out_sum, 10L * 1000L * 999L / 2L);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

