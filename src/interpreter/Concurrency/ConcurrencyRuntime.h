#ifndef MINISWIFT_CONCURRENCY_RUNTIME_H
#define MINISWIFT_CONCURRENCY_RUNTIME_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <queue>
#include <vector>
#include <functional>
#include <atomic>
#include <memory>
#include <coroutine>
#include <chrono>
#include <unordered_map>
#include <exception>
#include <any>

namespace miniswift {

// Forward declarations
class Task;
class Actor;
class TaskGroup;
class ThreadPool;
class ConcurrencyRuntime;

// Task priority levels
enum class TaskPriority {
    Background = 0,
    Utility = 1,
    Default = 2,
    UserInitiated = 3,
    UserInteractive = 4
};

// Task state
enum class TaskState {
    Created,
    Running,
    Suspended,
    Completed,
    Cancelled,
    Failed
};

// Cancellation token for cooperative cancellation
class CancellationToken {
public:
    CancellationToken() : cancelled_(false) {}
    
    void cancel() {
        std::lock_guard<std::mutex> lock(mutex_);
        cancelled_ = true;
        cv_.notify_all();
    }
    
    bool is_cancelled() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cancelled_;
    }
    
    void check_cancellation() const {
        if (is_cancelled()) {
            throw std::runtime_error("Task was cancelled");
        }
    }
    
    template<typename Rep, typename Period>
    bool wait_for_cancellation(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, timeout, [this] { return cancelled_; });
    }
    
private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool cancelled_;
};

// Task handle for managing async operations
class TaskHandle {
public:
    TaskHandle(std::shared_ptr<Task> task) : task_(task) {}
    
    void cancel();
    bool is_cancelled() const;
    TaskState get_state() const;
    void wait();
    
    template<typename T>
    T get_result();
    
private:
    std::shared_ptr<Task> task_;
};

// Base Task class
class Task {
public:
    using TaskId = uint64_t;
    
    Task(TaskPriority priority = TaskPriority::Default)
        : id_(generate_id()), priority_(priority), state_(TaskState::Created),
          cancellation_token_(std::make_shared<CancellationToken>()) {}
    
    virtual ~Task() = default;
    
    TaskId get_id() const { return id_; }
    TaskPriority get_priority() const { return priority_; }
    TaskState get_state() const {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return state_;
    }
    
    void set_state(TaskState state) {
        std::lock_guard<std::mutex> lock(state_mutex_);
        state_ = state;
    }
    
    std::shared_ptr<CancellationToken> get_cancellation_token() const {
        return cancellation_token_;
    }
    
    void cancel() {
        cancellation_token_->cancel();
        set_state(TaskState::Cancelled);
    }
    
    virtual void execute() = 0;
    
protected:
    static TaskId generate_id() {
        static std::atomic<TaskId> counter{0};
        return ++counter;
    }
    
    TaskId id_;
    TaskPriority priority_;
    mutable std::mutex state_mutex_;
    TaskState state_;
    std::shared_ptr<CancellationToken> cancellation_token_;
};

// Function-based task
template<typename Func, typename... Args>
class FunctionTask : public Task {
public:
    using ReturnType = std::invoke_result_t<Func, Args...>;
    
    FunctionTask(TaskPriority priority, Func&& func, Args&&... args)
        : Task(priority), func_(std::forward<Func>(func)), 
          args_(std::forward<Args>(args)...) {}
    
    void execute() override {
        try {
            set_state(TaskState::Running);
            if constexpr (std::is_void_v<ReturnType>) {
                std::apply(func_, args_);
                promise_.set_value();
            } else {
                auto result = std::apply(func_, args_);
                promise_.set_value(result);
            }
            set_state(TaskState::Completed);
        } catch (...) {
            promise_.set_exception(std::current_exception());
            set_state(TaskState::Failed);
        }
    }
    
    auto get_future() {
        if constexpr (std::is_void_v<ReturnType>) {
            return promise_.get_future();
        } else {
            return promise_.get_future();
        }
    }
    
private:
    Func func_;
    std::tuple<Args...> args_;
    
    std::conditional_t<std::is_void_v<ReturnType>,
                      std::promise<void>,
                      std::promise<ReturnType>> promise_;
};

// Thread pool for executing tasks
class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { worker_thread(); });
        }
    }
    
    ~ThreadPool() {
        shutdown();
    }
    
    void submit_task(std::shared_ptr<Task> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("ThreadPool is shutting down");
            }
            task_queue_.push(task);
        }
        condition_.notify_one();
    }
    
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    size_t get_thread_count() const {
        return workers_.size();
    }
    
    size_t get_pending_tasks() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return task_queue_.size();
    }
    
private:
    void worker_thread() {
        while (true) {
            std::shared_ptr<Task> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { return stop_ || !task_queue_.empty(); });
                
                if (stop_ && task_queue_.empty()) {
                    break;
                }
                
                task = task_queue_.front();
                task_queue_.pop();
            }
            
            if (task && task->get_state() != TaskState::Cancelled) {
                task->execute();
            }
        }
    }
    
    std::vector<std::thread> workers_;
    std::queue<std::shared_ptr<Task>> task_queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

// Task Group for structured concurrency
class TaskGroup {
public:
    TaskGroup() = default;
    ~TaskGroup() {
        wait_for_all();
    }
    
    template<typename Func, typename... Args>
    auto add_task(TaskPriority priority, Func&& func, Args&&... args) {
        auto task = std::make_shared<FunctionTask<Func, Args...>>(
            priority, std::forward<Func>(func), std::forward<Args>(args)...);
        
        {
            std::lock_guard<std::mutex> lock(tasks_mutex_);
            tasks_.push_back(task);
        }
        
        // Will be implemented after ConcurrencyRuntime is fully defined
        // ConcurrencyRuntime::get_instance().submit_task(task);
        return task->get_future();
    }
    
    void cancel_all() {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        for (auto& task : tasks_) {
            task->cancel();
        }
    }
    
    void wait_for_all() {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        for (auto& task : tasks_) {
            while (task->get_state() == TaskState::Running ||
                   task->get_state() == TaskState::Created) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
    
    size_t get_task_count() const {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        return tasks_.size();
    }
    
private:
    std::vector<std::shared_ptr<Task>> tasks_;
    mutable std::mutex tasks_mutex_;
};

// Actor for isolated state management
class Actor {
public:
    using ActorId = uint64_t;
    
    Actor() : id_(generate_id()), message_queue_(), processing_(false) {}
    virtual ~Actor() = default;
    
    ActorId get_id() const { return id_; }
    
    template<typename Message>
    void send_message(Message&& message) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            message_queue_.push([this, msg = std::forward<Message>(message)]() {
                this->handle_message(msg);
            });
        }
        
        process_messages();
    }
    
protected:
    virtual void handle_message(const std::any& message) = 0;
    
private:
    static ActorId generate_id() {
        static std::atomic<ActorId> counter{0};
        return ++counter;
    }
    
    void process_messages() {
        bool expected = false;
        if (!processing_.compare_exchange_strong(expected, true)) {
            return; // Already processing
        }
        
        // Will be implemented after ConcurrencyRuntime is fully defined
        // ConcurrencyRuntime::get_instance().submit_task(...);
    }
    
    ActorId id_;
    std::queue<std::function<void()>> message_queue_;
    std::mutex queue_mutex_;
    std::atomic<bool> processing_;
};

// Main concurrency runtime singleton
class ConcurrencyRuntime {
public:
    static ConcurrencyRuntime& get_instance() {
        static ConcurrencyRuntime instance;
        return instance;
    }
    
    void initialize(size_t num_threads = std::thread::hardware_concurrency()) {
        if (!thread_pool_) {
            thread_pool_ = std::make_unique<ThreadPool>(num_threads);
        }
    }
    
    void shutdown() {
        if (thread_pool_) {
            thread_pool_->shutdown();
            thread_pool_.reset();
        }
    }
    
    template<typename Func, typename... Args>
    auto async(TaskPriority priority, Func&& func, Args&&... args) {
        auto task = std::make_shared<FunctionTask<Func, Args...>>(
            priority, std::forward<Func>(func), std::forward<Args>(args)...);
        
        submit_task(task);
        return task->get_future();
    }
    
    void submit_task(std::shared_ptr<Task> task) {
        if (!thread_pool_) {
            throw std::runtime_error("ConcurrencyRuntime not initialized");
        }
        thread_pool_->submit_task(task);
    }
    
    // Sleep function for tasks
    template<typename Rep, typename Period>
    static void sleep_for(const std::chrono::duration<Rep, Period>& duration) {
        std::this_thread::sleep_for(duration);
    }
    
    // Yield current thread
    static void yield() {
        std::this_thread::yield();
    }
    
    size_t get_thread_count() const {
        return thread_pool_ ? thread_pool_->get_thread_count() : 0;
    }
    
    size_t get_pending_tasks() const {
        return thread_pool_ ? thread_pool_->get_pending_tasks() : 0;
    }
    
private:
    ConcurrencyRuntime() = default;
    ~ConcurrencyRuntime() {
        shutdown();
    }
    
    ConcurrencyRuntime(const ConcurrencyRuntime&) = delete;
    ConcurrencyRuntime& operator=(const ConcurrencyRuntime&) = delete;
    
    std::unique_ptr<ThreadPool> thread_pool_;
};

#ifdef __cpp_impl_coroutine
// C++20 Coroutine support (only if available)
template<typename T>
struct AsyncTask {
    struct promise_type {
        T value_;
        std::exception_ptr exception_;
        
        AsyncTask get_return_object() {
            return AsyncTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        
        void return_value(T value) {
            value_ = std::move(value);
        }
        
        void unhandled_exception() {
            exception_ = std::current_exception();
        }
    };
    
    std::coroutine_handle<promise_type> coro_;
    
    AsyncTask(std::coroutine_handle<promise_type> coro) : coro_(coro) {}
    
    ~AsyncTask() {
        if (coro_) {
            coro_.destroy();
        }
    }
    
    AsyncTask(const AsyncTask&) = delete;
    AsyncTask& operator=(const AsyncTask&) = delete;
    
    AsyncTask(AsyncTask&& other) noexcept : coro_(other.coro_) {
        other.coro_ = {};
    }
    
    AsyncTask& operator=(AsyncTask&& other) noexcept {
        if (this != &other) {
            if (coro_) {
                coro_.destroy();
            }
            coro_ = other.coro_;
            other.coro_ = {};
        }
        return *this;
    }
    
    T get_result() {
        if (!coro_.done()) {
            throw std::runtime_error("Coroutine not completed");
        }
        
        if (coro_.promise().exception_) {
            std::rethrow_exception(coro_.promise().exception_);
        }
        
        return std::move(coro_.promise().value_);
    }
    
    bool is_ready() const {
        return coro_.done();
    }
};

// Awaitable for async operations
template<typename T>
struct Awaitable {
    std::future<T> future_;
    
    Awaitable(std::future<T> future) : future_(std::move(future)) {}
    
    bool await_ready() {
        return future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }
    
    void await_suspend(std::coroutine_handle<> handle) {
        // Will be implemented after ConcurrencyRuntime is fully defined
        // ConcurrencyRuntime::get_instance().async(TaskPriority::Default, [this, handle]() {
        //     future_.wait();
        //     handle.resume();
        // });
    }
    
    T await_resume() {
        return future_.get();
    }
};

// Helper functions for creating awaitables
template<typename T>
Awaitable<T> make_awaitable(std::future<T> future) {
    return Awaitable<T>(std::move(future));
}
#endif // __cpp_impl_coroutine

// Global convenience functions
void initialize_concurrency_runtime(size_t num_threads = std::thread::hardware_concurrency());
void shutdown_concurrency_runtime();

} // namespace miniswift

#endif // MINISWIFT_CONCURRENCY_RUNTIME_H