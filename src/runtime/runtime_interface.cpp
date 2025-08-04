#include "runtime_interface.h"
#include "../interpreter/Memory/ARC.h"
#include "../interpreter/Memory/MemorySafety.h"
#include "../interpreter/Memory/MemoryManager.h"
#include "../interpreter/Concurrency/ConcurrencyRuntime.h"
#include "../interpreter/Value.h"

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <cstring>
#include <thread>

using namespace miniswift;

// =============================================================================
// Global state and error handling
// =============================================================================

static swift_error_code_t g_last_error = SWIFT_ERROR_NONE;
static std::mutex g_error_mutex;
static bool g_runtime_initialized = false;

static void set_error(swift_error_code_t error) {
    std::lock_guard<std::mutex> lock(g_error_mutex);
    g_last_error = error;
}

static void clear_error_internal() {
    std::lock_guard<std::mutex> lock(g_error_mutex);
    g_last_error = SWIFT_ERROR_NONE;
}

// =============================================================================
// Memory Management (ARC) Implementation
// =============================================================================

void* swift_retain(void* object) {
    if (!object) return nullptr;
    
    try {
        ARCManager::getInstance().retain(object);
        return object;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
        return nullptr;
    }
}

void swift_release(void* object) {
    if (!object) return;
    
    try {
        ARCManager::getInstance().release(object);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
    }
}

int32_t swift_get_ref_count(void* object) {
    if (!object) return 0;
    
    try {
        return static_cast<int32_t>(ARCManager::getInstance().getRefCount(object));
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
        return 0;
    }
}

// Weak reference implementation - match header definition
struct swift_weak_ref {
    void* object;
    bool is_alive;
};

swift_weak_ref_t* swift_weak_ref_create(void* object) {
    if (!object) return nullptr;
    
    try {
        auto weak_ref = new swift_weak_ref_t();
        weak_ref->object = object;
        weak_ref->is_alive = true;
        return weak_ref;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
        return nullptr;
    }
}

void* swift_weak_ref_lock(swift_weak_ref_t* weak_ref) {
    if (!weak_ref || !weak_ref->is_alive) return nullptr;
    return weak_ref->object;
}

bool swift_weak_ref_is_alive(swift_weak_ref_t* weak_ref) {
    return weak_ref ? weak_ref->is_alive : false;
}

void swift_weak_ref_destroy(swift_weak_ref_t* weak_ref) {
    if (weak_ref) {
        delete weak_ref;
    }
}

// Unowned reference implementation - match header definition
struct swift_unowned_ref {
    void* object;
};

swift_unowned_ref_t* swift_unowned_ref_create(void* object) {
    if (!object) return nullptr;
    
    try {
        auto unowned_ref = new swift_unowned_ref_t();
        unowned_ref->object = object;
        return unowned_ref;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
        return nullptr;
    }
}

void* swift_unowned_ref_get(swift_unowned_ref_t* unowned_ref) {
    return unowned_ref ? unowned_ref->object : nullptr;
}

void swift_unowned_ref_destroy(swift_unowned_ref_t* unowned_ref) {
    if (unowned_ref) {
        delete unowned_ref;
    }
}

// Memory safety implementation
void swift_memory_safety_init(void) {
    try {
        // Note: MemorySafetyManager doesn't have initialize() method, using getInstance() directly
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
    }
}

void swift_memory_safety_shutdown(void) {
    try {
        // Note: MemorySafetyManager doesn't have shutdown() method, this is handled by destructor
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
    }
}

bool swift_is_valid_pointer(void* ptr) {
    if (!ptr) return false;
    
    try {
        return MemorySafetyManager::getInstance().isPointerValid(ptr);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
        return false;
    }
}

void swift_register_object(void* object, size_t size) {
    if (!object) return;
    
    try {
        MemorySafetyManager::getInstance().registerMemoryRegion(object, size);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
    }
}

void swift_unregister_object(void* object) {
    if (!object) return;
    
    try {
        MemorySafetyManager::getInstance().unregisterMemoryRegion(object);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
    }
}

// Memory manager implementation
void swift_memory_manager_init(void) {
    try {
        // MemoryManager uses getInstance() and doesn't need explicit initialization
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
    }
}

void swift_memory_manager_shutdown(void) {
    try {
        // MemoryManager uses getInstance() and doesn't need explicit shutdown
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
    }
}

void* swift_create_object(size_t size) {
    try {
        // Allocate raw memory instead of using createObject template
        void* ptr = ::operator new(size);
        return ptr;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
        return nullptr;
    }
}

void swift_destroy_object(void* object) {
    if (!object) return;
    
    try {
        // MemoryManager doesn't have destroyObject, objects are managed by ARCPtr
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
    }
}

void swift_gc_collect(void) {
    try {
        MemoryManager::getInstance().collectGarbage();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_MEMORY);
    }
}

// =============================================================================
// Concurrency Implementation
// =============================================================================

// Task handle wrapper
struct swift_task_handle {
    std::shared_ptr<TaskHandle> handle;
};

void swift_concurrency_init(size_t num_threads) {
    try {
        initialize_concurrency_runtime(num_threads);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
    }
}

void swift_concurrency_shutdown(void) {
    try {
        shutdown_concurrency_runtime();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
    }
}

// Task function wrapper
struct TaskFunctionWrapper {
    void (*func)(void*);
    void* args;
};

static void execute_task_function(TaskFunctionWrapper* wrapper) {
    if (wrapper && wrapper->func) {
        wrapper->func(wrapper->args);
    }
    delete wrapper;
}

swift_task_handle_t* swift_async(swift_task_priority_t priority, void (*func)(void*), void* args) {
    if (!func) return nullptr;
    
    try {
        auto wrapper = new TaskFunctionWrapper{func, args};
        TaskPriority taskPriority = static_cast<TaskPriority>(priority);
        
        auto future = ConcurrencyRuntime::get_instance().async(
            taskPriority,
            [wrapper]() { execute_task_function(wrapper); }
        );
        
        // Create a simple task handle (simplified implementation)
        auto task_handle = std::make_unique<swift_task_handle>();
        // Note: This is a simplified implementation
        // In a real implementation, you would need to properly wrap the future
        return task_handle.release();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
        return nullptr;
    }
}

void* swift_await(swift_task_handle_t* task) {
    if (!task) return nullptr;
    
    try {
        // Simplified implementation - in reality you would wait for the task
        // and return its result
        return nullptr;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
        return nullptr;
    }
}

swift_task_handle_t* swift_task_create(void (*func)(void*), void* args) {
    return swift_async(SWIFT_TASK_PRIORITY_DEFAULT, func, args);
}

void swift_task_cancel(swift_task_handle_t* task) {
    if (!task || !task->handle) return;
    
    try {
        task->handle->cancel();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
    }
}

bool swift_task_is_cancelled(swift_task_handle_t* task) {
    if (!task || !task->handle) return false;
    
    try {
        return task->handle->is_cancelled();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
        return false;
    }
}

swift_task_state_t swift_task_get_state(swift_task_handle_t* task) {
    if (!task || !task->handle) return SWIFT_TASK_STATE_FAILED;
    
    try {
        TaskState state = task->handle->get_state();
        return static_cast<swift_task_state_t>(state);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
        return SWIFT_TASK_STATE_FAILED;
    }
}

void swift_task_wait(swift_task_handle_t* task) {
    if (!task || !task->handle) return;
    
    try {
        task->handle->wait();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
    }
}

void swift_task_destroy(swift_task_handle_t* task) {
    if (task) {
        delete task;
    }
}

void swift_task_sleep(uint64_t milliseconds) {
    try {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
    }
}

void swift_task_yield(void) {
    try {
        std::this_thread::yield();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
    }
}

bool swift_task_check_cancellation(void) {
    // Simplified implementation - would need thread-local storage for current task
    return false;
}

bool swift_task_is_cancelled_current(void) {
    // Simplified implementation - would need thread-local storage for current task
    return false;
}

// Actor implementation (simplified)
struct swift_actor {
    // Simplified actor implementation
    std::mutex mutex;
};

swift_actor_t* swift_actor_create(void) {
    try {
        return new swift_actor();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
        return nullptr;
    }
}

void swift_actor_destroy(swift_actor_t* actor) {
    if (actor) {
        delete actor;
    }
}

void swift_actor_execute(swift_actor_t* actor, void (*func)(void*), void* args) {
    if (!actor || !func) return;
    
    try {
        std::lock_guard<std::mutex> lock(actor->mutex);
        func(args);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
    }
}

void swift_actor_wait_idle(swift_actor_t* actor) {
    if (!actor) return;
    
    try {
        // Simplified implementation - in reality would wait for all pending operations
        std::lock_guard<std::mutex> lock(actor->mutex);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
    }
}

// Task group implementation (simplified)
struct swift_task_group {
    std::vector<std::unique_ptr<swift_task_handle>> tasks;
    std::mutex mutex;
};

swift_task_group_t* swift_task_group_create(void) {
    try {
        return new swift_task_group();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
        return nullptr;
    }
}

void swift_task_group_destroy(swift_task_group_t* group) {
    if (group) {
        delete group;
    }
}

swift_task_handle_t* swift_task_group_add_task(swift_task_group_t* group, 
                                               swift_task_priority_t priority,
                                               void (*func)(void*), 
                                               void* args) {
    if (!group || !func) return nullptr;
    
    try {
        auto task = swift_async(priority, func, args);
        if (task) {
            std::lock_guard<std::mutex> lock(group->mutex);
            group->tasks.emplace_back(task);
        }
        return task;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
        return nullptr;
    }
}

void swift_task_group_wait_for_all(swift_task_group_t* group) {
    if (!group) return;
    
    try {
        std::lock_guard<std::mutex> lock(group->mutex);
        for (auto& task : group->tasks) {
            if (task) {
                swift_task_wait(task.get());
            }
        }
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_CONCURRENCY);
    }

}

size_t swift_task_group_get_task_count(swift_task_group_t* group) {
    if (!group) return 0;
    std::lock_guard<std::mutex> lock(group->mutex);
    return group->tasks.size();
}

// =============================================================================
// AsyncSequence Implementation
// =============================================================================

struct swift_async_sequence {
    void (*generator)(void*);
    void* context;
    bool is_active;
};

struct swift_async_iterator {
    swift_async_sequence_t* sequence;
    size_t current_index;
    bool has_more;
};

swift_async_sequence_t* swift_async_sequence_create(void (*generator)(void*), void* context) {
    swift_async_sequence_t* sequence = (swift_async_sequence_t*)malloc(sizeof(swift_async_sequence_t));
    if (!sequence) return nullptr;
    
    sequence->generator = generator;
    sequence->context = context;
    sequence->is_active = true;
    
    return sequence;
}

void swift_async_sequence_destroy(swift_async_sequence_t* sequence) {
    if (sequence) {
        sequence->is_active = false;
        free(sequence);
    }
}

swift_async_iterator_t* swift_async_sequence_make_iterator(swift_async_sequence_t* sequence) {
    if (!sequence) return nullptr;
    
    swift_async_iterator_t* iterator = (swift_async_iterator_t*)malloc(sizeof(swift_async_iterator_t));
    if (!iterator) return nullptr;
    
    iterator->sequence = sequence;
    iterator->current_index = 0;
    iterator->has_more = true;
    
    return iterator;
}

void* swift_async_iterator_next(swift_async_iterator_t* iterator) {
    if (!iterator || !iterator->has_more) return nullptr;
    
    // 模拟异步迭代
    if (iterator->current_index >= 10) {
        iterator->has_more = false;
        return nullptr;
    }
    
    void* result = malloc(sizeof(int));
    *(int*)result = (int)iterator->current_index++;
    
    return result;
}

bool swift_async_iterator_has_next(swift_async_iterator_t* iterator) {
    return iterator && iterator->has_more;
}

void swift_async_iterator_destroy(swift_async_iterator_t* iterator) {
    if (iterator) {
        free(iterator);
    }
}

// =============================================================================
// Async Let Implementation
// =============================================================================

struct swift_async_let {
    swift_task_handle_t* task;
    void* result;
    bool is_ready;
    std::mutex mutex;
};

swift_async_let_t* swift_async_let_create(void (*func)(void*), void* args) {
    swift_async_let_t* async_let = (swift_async_let_t*)malloc(sizeof(swift_async_let_t));
    if (!async_let) return nullptr;
    
    async_let->task = swift_task_create(func, args);
    async_let->result = nullptr;
    async_let->is_ready = false;
    
    return async_let;
}

void* swift_async_let_await(swift_async_let_t* async_let) {
    if (!async_let) return nullptr;
    
    if (!async_let->is_ready) {
        async_let->result = swift_await(async_let->task);
        async_let->is_ready = true;
    }
    
    return async_let->result;
}

bool swift_async_let_is_ready(swift_async_let_t* async_let) {
    return async_let && async_let->is_ready;
}

void swift_async_let_destroy(swift_async_let_t* async_let) {
    if (async_let) {
        if (async_let->task) {
            swift_task_destroy(async_let->task);
        }
        free(async_let);
    }
}

// =============================================================================
// For-await-in Loop Implementation
// =============================================================================

swift_for_await_context_t* swift_for_await_begin(swift_async_sequence_t* sequence) {
    if (!sequence) return nullptr;
    
    swift_for_await_context_t* context = (swift_for_await_context_t*)malloc(sizeof(swift_for_await_context_t));
    if (!context) return nullptr;
    
    context->iterator = swift_async_sequence_make_iterator(sequence);
    context->is_active = true;
    
    return context;
}

void* swift_for_await_next(swift_for_await_context_t* context) {
    if (!context || !context->is_active) return nullptr;
    
    return swift_async_iterator_next(context->iterator);
}

bool swift_for_await_has_next(swift_for_await_context_t* context) {
    if (!context || !context->is_active) return false;
    
    return swift_async_iterator_has_next(context->iterator);
}

void swift_for_await_end(swift_for_await_context_t* context) {
    if (context) {
        context->is_active = false;
        if (context->iterator) {
            swift_async_iterator_destroy(context->iterator);
        }
        free(context);
    }
}

// =============================================================================
// Swift Type System Implementation
// =============================================================================

// String implementation
swift_string_t* swift_string_create(const char* cstr) {
    if (!cstr) return nullptr;
    
    try {
        auto str = new swift_string_t();
        str->length = strlen(cstr);
        str->capacity = str->length + 1;
        str->data = new char[str->capacity];
        strcpy(str->data, cstr);
        return str;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
        return nullptr;
    }
}

swift_string_t* swift_string_create_with_length(const char* data, size_t length) {
    if (!data) return nullptr;
    
    try {
        auto str = new swift_string_t();
        str->length = length;
        str->capacity = length + 1;
        str->data = new char[str->capacity];
        memcpy(str->data, data, length);
        str->data[length] = '\0';
        return str;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
        return nullptr;
    }
}

void swift_string_destroy(swift_string_t* str) {
    if (str) {
        delete[] str->data;
        delete str;
    }
}

const char* swift_string_get_cstring(swift_string_t* str) {
    return str ? str->data : nullptr;
}

size_t swift_string_get_length(swift_string_t* str) {
    return str ? str->length : 0;
}

// Array implementation
swift_array_t* swift_array_create(size_t element_size, size_t initial_capacity) {
    try {
        auto array = new swift_array_t();
        array->element_size = element_size;
        array->count = 0;
        array->capacity = initial_capacity;
        array->data = malloc(element_size * initial_capacity);
        return array;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
        return nullptr;
    }
}

void swift_array_destroy(swift_array_t* array) {
    if (array) {
        free(array->data);
        delete array;
    }
}

void swift_array_append(swift_array_t* array, const void* element) {
    if (!array || !element) return;
    
    try {
        if (array->count >= array->capacity) {
            // Resize array
            array->capacity *= 2;
            array->data = realloc(array->data, array->element_size * array->capacity);
        }
        
        char* dest = static_cast<char*>(array->data) + (array->count * array->element_size);
        memcpy(dest, element, array->element_size);
        array->count++;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
    }
}

void* swift_array_get(swift_array_t* array, size_t index) {
    if (!array || index >= array->count) return nullptr;
    
    try {
        char* data = static_cast<char*>(array->data);
        return data + (index * array->element_size);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
        return nullptr;
    }
}

size_t swift_array_get_count(swift_array_t* array) {
    return array ? array->count : 0;
}

// Dictionary implementation (simplified)
struct swift_dictionary {
    std::unordered_map<std::string, void*> data;
    std::mutex mutex;
};

swift_dictionary_t* swift_dict_create(size_t initial_capacity) {
    try {
        auto dict = new swift_dictionary();
        dict->data.reserve(initial_capacity);
        return dict;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
        return nullptr;
    }
}

void swift_dict_destroy(swift_dictionary_t* dict) {
    if (dict) {
        delete dict;
    }
}

void swift_dict_set(swift_dictionary_t* dict, const void* key, const void* value) {
    if (!dict || !key) return;
    
    try {
        std::lock_guard<std::mutex> lock(dict->mutex);
        std::string key_str(static_cast<const char*>(key));
        dict->data[key_str] = const_cast<void*>(value);
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
    }
}

void* swift_dict_get(swift_dictionary_t* dict, const void* key) {
    if (!dict || !key) return nullptr;
    
    try {
        std::lock_guard<std::mutex> lock(dict->mutex);
        std::string key_str(static_cast<const char*>(key));
        auto it = dict->data.find(key_str);
        return (it != dict->data.end()) ? it->second : nullptr;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
        return nullptr;
    }
}

bool swift_dict_contains_key(swift_dictionary_t* dict, const void* key) {
    if (!dict || !key) return false;
    
    try {
        std::lock_guard<std::mutex> lock(dict->mutex);
        std::string key_str(static_cast<const char*>(key));
        return dict->data.find(key_str) != dict->data.end();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
        return false;
    }
}

size_t swift_dict_get_count(swift_dictionary_t* dict) {
    if (!dict) return 0;
    
    try {
        std::lock_guard<std::mutex> lock(dict->mutex);
        return dict->data.size();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
        return 0;
    }
}

// Optional implementation
swift_optional_t* swift_optional_create(void* value, bool has_value) {
    try {
        auto optional = new swift_optional_t();
        optional->has_value = has_value;
        optional->value = value;
        return optional;
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_TYPE);
        return nullptr;
    }
}

void swift_optional_destroy(swift_optional_t* optional) {
    if (optional) {
        delete optional;
    }
}

bool swift_optional_has_value(swift_optional_t* optional) {
    return optional ? optional->has_value : false;
}

void* swift_optional_get_value(swift_optional_t* optional) {
    return (optional && optional->has_value) ? optional->value : nullptr;
}

// =============================================================================
// Error Handling Implementation
// =============================================================================

swift_error_code_t swift_get_last_error(void) {
    std::lock_guard<std::mutex> lock(g_error_mutex);
    return g_last_error;
}

const char* swift_get_error_message(swift_error_code_t error_code) {
    switch (error_code) {
        case SWIFT_ERROR_NONE:
            return "No error";
        case SWIFT_ERROR_MEMORY:
            return "Memory management error";
        case SWIFT_ERROR_CONCURRENCY:
            return "Concurrency error";
        case SWIFT_ERROR_TYPE:
            return "Type system error";
        case SWIFT_ERROR_RUNTIME:
            return "Runtime error";
        default:
            return "Unknown error";
    }
}

void swift_clear_error(void) {
    clear_error_internal();
}

// =============================================================================
// Runtime Initialization Implementation
// =============================================================================

void swift_runtime_init(size_t concurrency_threads) {
    if (g_runtime_initialized) return;
    
    try {
        // Initialize memory management
        swift_memory_safety_init();
        swift_memory_manager_init();
        
        // Initialize concurrency
        swift_concurrency_init(concurrency_threads);
        
        g_runtime_initialized = true;
        clear_error_internal();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_RUNTIME);
    }
}

void swift_runtime_shutdown(void) {
    if (!g_runtime_initialized) return;
    
    try {
        // Shutdown concurrency
        swift_concurrency_shutdown();
        
        // Shutdown memory management
        swift_memory_manager_shutdown();
        swift_memory_safety_shutdown();
        
        g_runtime_initialized = false;
        clear_error_internal();
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_RUNTIME);
    }
}

void swift_runtime_init_with_config(const swift_runtime_config_t* config) {
    if (!config) {
        swift_runtime_init(std::thread::hardware_concurrency());
        return;
    }
    
    swift_runtime_init(config->concurrency_threads);
}

swift_runtime_config_t swift_runtime_get_default_config(void) {
    swift_runtime_config_t config;
    config.concurrency_threads = std::thread::hardware_concurrency();
    config.enable_memory_safety = true;
    config.enable_arc = true;
    config.enable_gc = true;
    config.gc_threshold = 1024 * 1024; // 1MB
    return config;
}

swift_runtime_stats_t swift_runtime_get_stats(void) {
    swift_runtime_stats_t stats = {};
    
    try {
        // Get memory statistics
        auto memStats = MemoryManager::getInstance().getMemoryStats();
        stats.total_objects = memStats.totalManagedObjects;
        stats.active_objects = memStats.totalStrongRefs;
        stats.total_memory = memStats.memoryRegions;
        
        // Get concurrency statistics
        auto& runtime = ConcurrencyRuntime::get_instance();
        stats.thread_count = runtime.get_thread_count();
        stats.active_tasks = runtime.get_pending_tasks();
        stats.completed_tasks = 0; // Would need to track this
    } catch (const std::exception& e) {
        set_error(SWIFT_ERROR_RUNTIME);
    }
    
    return stats;
}

void swift_runtime_print_stats(void) {
    auto stats = swift_runtime_get_stats();
    
    std::cout << "=== MiniSwift Runtime Statistics ===\n";
    std::cout << "Memory:";
    std::cout << "  Total objects: " << stats.total_objects << "\n";
    std::cout << "  Active objects: " << stats.active_objects << "\n";
    std::cout << "  Total memory: " << stats.total_memory << " bytes\n";
    std::cout << "Concurrency:\n";
    std::cout << "  Thread count: " << stats.thread_count << "\n";
    std::cout << "  Active tasks: " << stats.active_tasks << "\n";
    std::cout << "  Completed tasks: " << stats.completed_tasks << "\n";
    std::cout << "====================================\n";
}