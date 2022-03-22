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

            Queue(size_t capacity) : capacity{capacity} {
                std::deque<T> d{};
                d.resize(capacity);
                d.clear();
                _queue = std::queue<T>{d};
            }

            size_t size() { return _queue.size(); }
            bool is_empty() { return size() == 0; }
            bool is_full() { return size() == capacity; }

            bool enqueue(const T& value)
            {
                return enqueue(value, std::chrono::seconds(inf_timeout));    // 100 years
            }

            template<typename Rep, typename Period>
            bool enqueue(const T& value, const std::chrono::duration<Rep, Period> &timeout)
            {
                std::unique_lock<std::timed_mutex> lock(mtx, timeout);
                if (!lock.owns_lock()) throw std::runtime_error{"Failed acquiring lock."};
                
                auto succeded = dequeue_cv.wait_for(lock, timeout, [this]() { return !this->is_full(); });
                if (!succeded) return false;

                _queue.push(value);
                enqueue_cv.notify_one();
                return true;
            }

            std::unique_ptr<T> dequeue()
            {
                return dequeue(std::chrono::seconds(inf_timeout));    // 100 years
            }

            template<typename Rep, typename Period>
            void dequeue(T *out, const std::chrono::duration<Rep, Period> &timeout)
            {
                std::unique_lock<std::timed_mutex> lock(mtx, timeout);
                if (!lock.owns_lock()) throw std::runtime_error{"Failed acquiring lock."};

                auto succeded = enqueue_cv.wait_for(lock, timeout, [this]() { return !this->is_empty(); });
                if (!succeded) {
                    out = nullptr;
                    return;
                }

                *out = _queue.front();
                _queue.pop();
                dequeue_cv.notify_one();
            }

            template<typename Rep, typename Period>
            std::unique_ptr<T> dequeue(const std::chrono::duration<Rep, Period> &timeout)
            {
                std::unique_lock<std::timed_mutex> lock(mtx, timeout);
                if (!lock.owns_lock()) throw std::runtime_error{"Failed acquiring lock."};

                auto succeded = enqueue_cv.wait_for(lock, timeout, [this]() { return !this->is_empty(); });
                if (!succeded) {
                    return std::unique_ptr<T>{nullptr};
                }

                auto re = std::make_unique<T>(_queue.front());
                _queue.pop();
                dequeue_cv.notify_one();
                return re;
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
