#ifndef THREAD_SAFE_COLLECTIONS_H_
#define THREAD_SAFE_COLLECTIONS_H_


#include <queue>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <exception>


namespace thread_safe
{

    template<typename T>
    class Queue
    {
        public:
            
            class QueueFull : public std::exception {};
            class QueueEmpty : public std::exception {};

            Queue(size_t capacity) : capacity{capacity} {
                std::deque<T> d{};
                d.resize(capacity);
                d.clear();
                _queue = std::queue<T>{d};
            }

            size_t size() { return _queue.size(); }
            bool is_empty() { return size() == 0; }
            bool is_full() { return size() == capacity; }

            void enqueue(const T& value)
            {
                enqueue(value, std::chrono::seconds(inf_timeout));    // 100 years
            }

            template<typename Rep, typename Period>
            void enqueue(const T& value, const std::chrono::duration<Rep, Period> &timeout)
            {
                std::unique_lock<std::timed_mutex> lock(mtx, timeout);
                if (!lock.owns_lock()) throw std::runtime_error{"Failed acquiring lock."};
                
                auto succeded = dequeue_cv.wait_for(lock, timeout, [this]() { return !this->is_full(); });
                if (!succeded) throw QueueFull{};

                _queue.push(value);
                enqueue_cv.notify_one();
            }

            std::unique_ptr<T> dequeue()
            {
                return dequeue(std::chrono::seconds(inf_timeout));    // 100 years
            }

            void dequeue(T *out)
            {
                dequeue(out, std::chrono::seconds(inf_timeout));    // 100 years
            }

            template<typename Rep, typename Period>
            void dequeue(T *out, const std::chrono::duration<Rep, Period> &timeout)
            {
                std::unique_lock<std::timed_mutex> lock(mtx, timeout);
                if (!lock.owns_lock()) throw std::runtime_error{"Failed acquiring lock."};

                auto succeded = enqueue_cv.wait_for(lock, timeout, [this]() { return !this->is_empty(); });
                if (!succeded) throw QueueEmpty{};

                *out = _queue.front();
                _queue.pop();
                dequeue_cv.notify_one();
            }

            template<typename Rep, typename Period>
            std::unique_ptr<T> dequeue(const std::chrono::duration<Rep, Period> &timeout)
            {
                T *out;
                dequeue(out, timeout);
                return std::make_unique<T>(*out);
            }

        private:
            const int64_t inf_timeout{3600L * 24L * 365L * 100L};
            size_t capacity;
            std::queue<T> _queue;
            std::timed_mutex mtx{};
            std::condition_variable_any dequeue_cv{};
            std::condition_variable_any enqueue_cv{};
    };
}

#endif /* THREAD_SAFE_COLLECTIONS_H_ */
