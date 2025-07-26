#include "../src/interpreter/Concurrency/ConcurrencyRuntime.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>

using namespace miniswift;
using namespace std::chrono_literals;

// Forward declarations for global functions
void initialize_concurrency_runtime(size_t num_threads);
void shutdown_concurrency_runtime();

// Example Actor implementation
class CounterActor : public Actor {
public:
    CounterActor() : counter_(0) {}
    
    void increment() {
        send_message("increment");
    }
    
    void decrement() {
        send_message("decrement");
    }
    
    void get_value() {
        send_message("get_value");
    }
    
protected:
    void handle_message(const std::any& message) override {
        try {
            std::string msg = std::any_cast<std::string>(message);
            
            if (msg == "increment") {
                counter_++;
                std::cout << "Counter incremented to: " << counter_ << std::endl;
            } else if (msg == "decrement") {
                counter_--;
                std::cout << "Counter decremented to: " << counter_ << std::endl;
            } else if (msg == "get_value") {
                std::cout << "Current counter value: " << counter_ << std::endl;
            }
        } catch (const std::bad_any_cast& e) {
            std::cerr << "Invalid message type: " << e.what() << std::endl;
        }
    }
    
private:
    int counter_;
};

// Example function for parallel computation
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Example of CPU-intensive task
void cpu_intensive_task(int task_id, int iterations) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);
    
    double result = 0.0;
    for (int i = 0; i < iterations; ++i) {
        result += std::sqrt(dis(gen));
    }
    
    std::cout << "Task " << task_id << " completed with result: " << result << std::endl;
}

int main() {
    std::cout << "=== MiniSwift Concurrency Runtime Test ===\n\n";
    
    // Initialize the concurrency runtime
    initialize_concurrency_runtime(4);
    
    auto& runtime = ConcurrencyRuntime::get_instance();
    
    std::cout << "Runtime initialized with " << runtime.get_thread_count() << " threads\n\n";
    
    // Test 1: Basic async tasks
    std::cout << "Test 1: Basic Async Tasks\n";
    std::cout << "------------------------\n";
    
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 5; ++i) {
        auto future = runtime.async(TaskPriority::Default, cpu_intensive_task, i, 10000);
        futures.push_back(std::move(future));
    }
    
    // Wait for all tasks to complete
    for (auto& future : futures) {
        future.wait();
    }
    
    std::cout << "\nAll basic async tasks completed!\n\n";
    
    // Test 2: Task Group
    std::cout << "Test 2: Task Group\n";
    std::cout << "------------------\n";
    
    {
        TaskGroup group;
        
        // Add multiple tasks to the group
        for (int i = 0; i < 3; ++i) {
            group.add_task(TaskPriority::Default, [i]() {
                std::cout << "Task group task " << i << " starting\n";
                std::this_thread::sleep_for(100ms);
                std::cout << "Task group task " << i << " completed\n";
            });
        }
        
        std::cout << "Task group has " << group.get_task_count() << " tasks\n";
        
        // TaskGroup destructor will wait for all tasks
    }
    
    std::cout << "\nTask group completed!\n\n";
    
    // Test 3: Actor Model
    std::cout << "Test 3: Actor Model\n";
    std::cout << "-------------------\n";
    
    CounterActor counter;
    
    // Send messages to the actor
    for (int i = 0; i < 5; ++i) {
        counter.increment();
    }
    
    for (int i = 0; i < 2; ++i) {
        counter.decrement();
    }
    
    counter.get_value();
    
    // Give some time for actor messages to process
    std::this_thread::sleep_for(500ms);
    
    std::cout << "\nActor test completed!\n\n";
    
    // Test 4: Parallel computation with futures
    std::cout << "Test 4: Parallel Fibonacci Computation\n";
    std::cout << "--------------------------------------\n";
    
    std::vector<std::future<int>> fib_futures;
    std::vector<int> fib_numbers = {35, 36, 37, 38, 39};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int n : fib_numbers) {
        auto future = runtime.async(TaskPriority::UserInteractive, fibonacci, n);
        fib_futures.push_back(std::move(future));
    }
    
    // Collect results
    for (size_t i = 0; i < fib_futures.size(); ++i) {
        int result = fib_futures[i].get();
        std::cout << "fibonacci(" << fib_numbers[i] << ") = " << result << std::endl;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\nParallel fibonacci computation took: " << duration.count() << " ms\n\n";
    
    // Test 5: Task priorities
    std::cout << "Test 5: Task Priorities\n";
    std::cout << "-----------------------\n";
    
    // Submit tasks with different priorities
    auto low_priority = runtime.async(TaskPriority::Background, []() {
        std::cout << "Low priority task executed\n";
        std::this_thread::sleep_for(100ms);
    });
    
    auto high_priority = runtime.async(TaskPriority::UserInteractive, []() {
        std::cout << "High priority task executed\n";
        std::this_thread::sleep_for(50ms);
    });
    
    auto default_priority = runtime.async(TaskPriority::Default, []() {
        std::cout << "Default priority task executed\n";
        std::this_thread::sleep_for(75ms);
    });
    
    // Wait for all priority tasks
    low_priority.wait();
    high_priority.wait();
    default_priority.wait();
    
    std::cout << "\nPriority test completed!\n\n";
    
    // Test 6: Runtime statistics
    std::cout << "Test 6: Runtime Statistics\n";
    std::cout << "--------------------------\n";
    
    std::cout << "Current pending tasks: " << runtime.get_pending_tasks() << std::endl;
    std::cout << "Thread count: " << runtime.get_thread_count() << std::endl;
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << std::endl;
    
    std::cout << "\n=== All tests completed successfully! ===\n";
    
    // Shutdown the runtime
    shutdown_concurrency_runtime();
    
    return 0;
}