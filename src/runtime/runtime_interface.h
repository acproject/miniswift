#ifndef MINISWIFT_RUNTIME_INTERFACE_H
#define MINISWIFT_RUNTIME_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Memory Management (ARC) Interface
// =============================================================================

// Object reference counting
void* swift_retain(void* object);
void swift_release(void* object);
int32_t swift_get_ref_count(void* object);

// Weak and unowned references
typedef struct {
    void* object;
    bool is_alive;
} swift_weak_ref_t;

typedef struct {
    void* object;
} swift_unowned_ref_t;

swift_weak_ref_t* swift_weak_ref_create(void* object);
void* swift_weak_ref_lock(swift_weak_ref_t* weak_ref);
bool swift_weak_ref_is_alive(swift_weak_ref_t* weak_ref);
void swift_weak_ref_destroy(swift_weak_ref_t* weak_ref);

swift_unowned_ref_t* swift_unowned_ref_create(void* object);
void* swift_unowned_ref_get(swift_unowned_ref_t* unowned_ref);
void swift_unowned_ref_destroy(swift_unowned_ref_t* unowned_ref);

// Memory safety
void swift_memory_safety_init(void);
void swift_memory_safety_shutdown(void);
bool swift_is_valid_pointer(void* ptr);
void swift_register_object(void* object, size_t size);
void swift_unregister_object(void* object);

// Memory manager
void swift_memory_manager_init(void);
void swift_memory_manager_shutdown(void);
void* swift_create_object(size_t size);
void swift_destroy_object(void* object);
void swift_gc_collect(void);

// =============================================================================
// Concurrency Interface
// =============================================================================

// Task types and states
typedef enum {
    SWIFT_TASK_PRIORITY_BACKGROUND = 0,
    SWIFT_TASK_PRIORITY_UTILITY = 1,
    SWIFT_TASK_PRIORITY_DEFAULT = 2,
    SWIFT_TASK_PRIORITY_USER_INITIATED = 3,
    SWIFT_TASK_PRIORITY_USER_INTERACTIVE = 4
} swift_task_priority_t;

typedef enum {
    SWIFT_TASK_STATE_CREATED = 0,
    SWIFT_TASK_STATE_RUNNING = 1,
    SWIFT_TASK_STATE_SUSPENDED = 2,
    SWIFT_TASK_STATE_COMPLETED = 3,
    SWIFT_TASK_STATE_CANCELLED = 4,
    SWIFT_TASK_STATE_FAILED = 5
} swift_task_state_t;

// Opaque task handle
typedef struct swift_task_handle swift_task_handle_t;

// Concurrency runtime initialization
void swift_concurrency_init(size_t num_threads);
void swift_concurrency_shutdown(void);

// Task creation and management
swift_task_handle_t* swift_async(swift_task_priority_t priority, void (*func)(void*), void* args);
void* swift_await(swift_task_handle_t* task);
swift_task_handle_t* swift_task_create(void (*func)(void*), void* args);

// Task control
void swift_task_cancel(swift_task_handle_t* task);
bool swift_task_is_cancelled(swift_task_handle_t* task);
swift_task_state_t swift_task_get_state(swift_task_handle_t* task);
void swift_task_wait(swift_task_handle_t* task);
void swift_task_destroy(swift_task_handle_t* task);

// Task utilities
void swift_task_sleep(uint64_t milliseconds);
void swift_task_yield(void);
bool swift_task_check_cancellation(void);
bool swift_task_is_cancelled_current(void);

// Actor support
typedef struct swift_actor swift_actor_t;

swift_actor_t* swift_actor_create(void);
void swift_actor_destroy(swift_actor_t* actor);
void swift_actor_execute(swift_actor_t* actor, void (*func)(void*), void* args);
void swift_actor_wait_idle(swift_actor_t* actor);

// Task group support
typedef struct swift_task_group swift_task_group_t;

swift_task_group_t* swift_task_group_create(void);
void swift_task_group_destroy(swift_task_group_t* group);
swift_task_handle_t* swift_task_group_add_task(swift_task_group_t* group, 
                                               swift_task_priority_t priority,
                                               void (*func)(void*), 
                                               void* args);
void swift_task_group_wait_for_all(swift_task_group_t* group);
size_t swift_task_group_get_task_count(swift_task_group_t* group);

// AsyncSequence support
typedef struct swift_async_sequence swift_async_sequence_t;
typedef struct swift_async_iterator swift_async_iterator_t;

swift_async_sequence_t* swift_async_sequence_create(void (*generator)(void*), void* context);
void swift_async_sequence_destroy(swift_async_sequence_t* sequence);
swift_async_iterator_t* swift_async_sequence_make_iterator(swift_async_sequence_t* sequence);
void* swift_async_iterator_next(swift_async_iterator_t* iterator);
bool swift_async_iterator_has_next(swift_async_iterator_t* iterator);
void swift_async_iterator_destroy(swift_async_iterator_t* iterator);

// Async let support
typedef struct swift_async_let swift_async_let_t;

swift_async_let_t* swift_async_let_create(void (*func)(void*), void* args);
void* swift_async_let_await(swift_async_let_t* async_let);
bool swift_async_let_is_ready(swift_async_let_t* async_let);
void swift_async_let_destroy(swift_async_let_t* async_let);

// For-await-in loop support
typedef struct {
    swift_async_iterator_t* iterator;
    bool is_active;
} swift_for_await_context_t;

swift_for_await_context_t* swift_for_await_begin(swift_async_sequence_t* sequence);
void* swift_for_await_next(swift_for_await_context_t* context);
bool swift_for_await_has_next(swift_for_await_context_t* context);
void swift_for_await_end(swift_for_await_context_t* context);

// =============================================================================
// Swift Type System Interface
// =============================================================================

// String support
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} swift_string_t;

swift_string_t* swift_string_create(const char* cstr);
swift_string_t* swift_string_create_with_length(const char* data, size_t length);
void swift_string_destroy(swift_string_t* str);
const char* swift_string_get_cstring(swift_string_t* str);
size_t swift_string_get_length(swift_string_t* str);

// Array support
typedef struct {
    void* data;
    size_t count;
    size_t capacity;
    size_t element_size;
} swift_array_t;

swift_array_t* swift_array_create(size_t element_size, size_t initial_capacity);
void swift_array_destroy(swift_array_t* array);
void swift_array_append(swift_array_t* array, const void* element);
void* swift_array_get(swift_array_t* array, size_t index);
size_t swift_array_get_count(swift_array_t* array);

// Dictionary support
typedef struct swift_dictionary swift_dictionary_t;

swift_dictionary_t* swift_dict_create(size_t initial_capacity);
void swift_dict_destroy(swift_dictionary_t* dict);
void swift_dict_set(swift_dictionary_t* dict, const void* key, const void* value);
void* swift_dict_get(swift_dictionary_t* dict, const void* key);
bool swift_dict_contains_key(swift_dictionary_t* dict, const void* key);
size_t swift_dict_get_count(swift_dictionary_t* dict);

// Optional support
typedef struct {
    bool has_value;
    void* value;
} swift_optional_t;

swift_optional_t* swift_optional_create(void* value, bool has_value);
void swift_optional_destroy(swift_optional_t* optional);
bool swift_optional_has_value(swift_optional_t* optional);
void* swift_optional_get_value(swift_optional_t* optional);

// =============================================================================
// Error Handling
// =============================================================================

typedef enum {
    SWIFT_ERROR_NONE = 0,
    SWIFT_ERROR_MEMORY = 1,
    SWIFT_ERROR_CONCURRENCY = 2,
    SWIFT_ERROR_TYPE = 3,
    SWIFT_ERROR_RUNTIME = 4
} swift_error_code_t;

swift_error_code_t swift_get_last_error(void);
const char* swift_get_error_message(swift_error_code_t error_code);
void swift_clear_error(void);

// =============================================================================
// Runtime Initialization
// =============================================================================

// Initialize the entire MiniSwift runtime
void swift_runtime_init(size_t concurrency_threads);
void swift_runtime_shutdown(void);

// Runtime configuration
typedef struct {
    size_t concurrency_threads;
    bool enable_memory_safety;
    bool enable_arc;
    bool enable_gc;
    size_t gc_threshold;
} swift_runtime_config_t;

void swift_runtime_init_with_config(const swift_runtime_config_t* config);
swift_runtime_config_t swift_runtime_get_default_config(void);

// Runtime statistics
typedef struct {
    size_t total_objects;
    size_t active_objects;
    size_t total_memory;
    size_t active_tasks;
    size_t completed_tasks;
    size_t thread_count;
} swift_runtime_stats_t;

swift_runtime_stats_t swift_runtime_get_stats(void);
void swift_runtime_print_stats(void);

#ifdef __cplusplus
}
#endif

#endif // MINISWIFT_RUNTIME_INTERFACE_H