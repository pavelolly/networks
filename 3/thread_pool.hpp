#pragma once

#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <vector>
#include <print>

#include "td.hpp"

template <typename...>
class Task;

template <>
class Task<> {
public:
    virtual void call() = 0;
    virtual ~Task() {};
};

template <typename Func, typename... Args>
class Task<Func, Args...> : public Task<> {
public:
    using CallableType  = std::decay_t<std::remove_reference_t<Func>>;
    using ReturnType    = std::invoke_result_t<CallableType, Args...>;

    using ArgsTuple     = std::tuple<std::remove_reference_t<Args>...>;

    using IndexSequence = std::index_sequence_for<Args...>;

    explicit Task(Func &&func, Args &&...args) : func(std::forward<Func>(func)),
                                                 args(std::forward<Args>(args)...)
    {}

    virtual void call() override {
        call(IndexSequence{});
    }
private:
    CallableType func;
    ArgsTuple args;

    template<size_t ...N>
    void call(std::index_sequence<N...>) {
        // we wanna move the values if possible (implies that generally task can be called only once)
        func(std::get<N>(std::move(args))...);
    }
};

// probably not needed with c++23
// template <typename Func, typename... Args>
// Task(Func &&func, Args &&...args) -> Task<Func, Args...>;

class ThreadPool {
public:
    ThreadPool(std::size_t num_threads) : pool(num_threads) {
        for (std::size_t i = 0; i < pool.size(); ++i) {
            pool[i] = std::thread(&ThreadPool::Run, this, i);
        }
    }

    ~ThreadPool() {
        std::unique_lock<std::mutex> lock(queue_mutex);

        Log("~ThreadPool waiting for empty queue (queue_tasks.size(): {})", queue_tasks.size());

        queue_access_cv.wait(lock, [this]() -> bool { return queue_tasks.empty(); });
        lock.unlock();
        
        Log("~ThreadPool is notifying threads to finish their last tasks (queue_tasks.size(): {})", queue_tasks.size());

        b_running = false;
        queue_access_cv.notify_all();

        Log("~ThreadPool is joining threads (queue_tasks.size(): {})", queue_tasks.size());

        for (std::size_t i = 0; i < pool.size(); ++i) {
            pool[i].join();
        }

        std::println("ThreadPool successfully destroyed");
    }

    bool Running() {
        return b_running.load(std::memory_order_acquire);
    }

    bool Paused() {
        return b_paused.load(std::memory_order_acquire);
    }

    template <typename Func, typename ...Args>
    void NewTask(Func &&func, Args &&...args) {
        queue_mutex.lock();

        auto p = std::make_unique<Task<Func, Args...>>(std::forward<Func>(func), std::forward<Args>(args)...);
        queue_tasks.push(std::move(p));

        Log("Added task");

        queue_mutex.unlock();

        queue_access_cv.notify_all();
    }

    template <class... Args>
    void Log(const std::format_string<Args...> fmt, Args&&... args) {
        if (b_logging.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(logging_mutex);
            std::print("[Thread Pool: INFO] ");
            std::println(fmt, std::forward<Args>(args)...);
        }
    }

    void SetLoggingFlag(bool value) {
        b_logging.store(value, std::memory_order_release);
    }

    void SetPause(bool value) {
        b_paused.store(value, std::memory_order_release);
    }
private:
    void Run(std::size_t tid) {
        Log("Thread {} started (running: {})", tid, b_running.load(std::memory_order_relaxed));

        while (Running()) {
            std::unique_lock<std::mutex> lock(queue_mutex);

            Log("Thread {} waiting", tid);

            queue_access_cv.wait(lock, [this]() { return !queue_tasks.empty() || !Running(); });

            if (!Running()) break;

            Log("Thread {} woke up (queue_tasks.size(): {})", tid, queue_tasks.size());

            auto task = std::move(queue_tasks.front());
            queue_tasks.pop();

            Log("Thread {} popped the task", tid);
            
            lock.unlock();
            queue_access_cv.notify_all();
                
            Log("Thread {} calling", tid);

            task->call();
            
            Log("Thread {} has done the task", tid);
        }
        Log("Thread {} ended", tid);
    }

    std::vector<std::thread> pool;
    std::queue<std::unique_ptr<Task<>>> queue_tasks;
    
    std::atomic<bool> b_running = true;
    std::atomic<bool> b_paused  = false;
    std::atomic<bool> b_logging = false;

    std::mutex logging_mutex;
    std::mutex queue_mutex;
    std::condition_variable queue_access_cv;
};