#pragma once

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <vector>
#include <stdexcept>
#include <atomic>
#include <type_traits>

namespace miniswift {

// 前向声明
struct Value;

// 内存安全异常类
class MemorySafetyException : public std::runtime_error {
public:
    explicit MemorySafetyException(const std::string& message)
        : std::runtime_error("Memory Safety Error: " + message) {}
};

class DanglingPointerException : public MemorySafetyException {
public:
    explicit DanglingPointerException(const std::string& message)
        : MemorySafetyException("Dangling Pointer: " + message) {}
};

class ArrayBoundsException : public MemorySafetyException {
public:
    explicit ArrayBoundsException(const std::string& message)
        : MemorySafetyException("Array Bounds: " + message) {}
};

class UseAfterFreeException : public MemorySafetyException {
public:
    explicit UseAfterFreeException(const std::string& message)
        : MemorySafetyException("Use After Free: " + message) {}
};

// 内存区域信息
struct MemoryRegion {
    void* start;
    size_t size;
    bool isValid;
    std::string debugInfo;
    
    MemoryRegion() : start(nullptr), size(0), isValid(false), debugInfo("") {}
    
    MemoryRegion(void* ptr, size_t sz, const std::string& info = "")
        : start(ptr), size(sz), isValid(true), debugInfo(info) {}
    
    bool contains(void* ptr) const {
        if (!isValid || !start || !ptr) return false;
        uintptr_t startAddr = reinterpret_cast<uintptr_t>(start);
        uintptr_t ptrAddr = reinterpret_cast<uintptr_t>(ptr);
        return ptrAddr >= startAddr && ptrAddr < startAddr + size;
    }
};

// 前向声明
class MemorySafetyManager;

// 安全指针包装器
template<typename T>
class SafePtr {
private:
    T* ptr;
    std::shared_ptr<std::atomic<bool>> isValid;
    std::string debugInfo;
    
public:
    SafePtr();
    explicit SafePtr(T* p, const std::string& info = "");
    SafePtr(const SafePtr& other);
    SafePtr(SafePtr&& other) noexcept;
    SafePtr& operator=(const SafePtr& other);
    SafePtr& operator=(SafePtr&& other) noexcept;
    ~SafePtr();
    
    T* get() const;
    T& operator*() const;
    T* operator->() const;
    explicit operator bool() const;
    bool operator==(const SafePtr& other) const;
    bool operator!=(const SafePtr& other) const;
    void reset(T* newPtr = nullptr, const std::string& info = "");
    
private:
    void checkValidity() const;
};

// 安全数组包装器
template<typename T>
class SafeArray {
private:
    std::vector<T> data;
    std::string debugInfo;
    
public:
    SafeArray() = default;
    
    explicit SafeArray(size_t size, const std::string& info = "")
        : data(size), debugInfo(info) {}
    
    SafeArray(size_t size, const T& value, const std::string& info = "")
        : data(size, value), debugInfo(info) {}
    
    SafeArray(std::initializer_list<T> init, const std::string& info = "")
        : data(init), debugInfo(info) {}
    
    T& operator[](size_t index) {
        checkBounds(index);
        return data[index];
    }
    
    const T& operator[](size_t index) const {
        checkBounds(index);
        return data[index];
    }
    
    T& at(size_t index) {
        checkBounds(index);
        return data[index];
    }
    
    const T& at(size_t index) const {
        checkBounds(index);
        return data[index];
    }
    
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    
    void push_back(const T& value) { data.push_back(value); }
    void push_back(T&& value) { data.push_back(std::move(value)); }
    
    void pop_back() {
        if (data.empty()) {
            throw ArrayBoundsException("pop_back on empty array: " + debugInfo);
        }
        data.pop_back();
    }
    
    void resize(size_t newSize) { data.resize(newSize); }
    void clear() { data.clear(); }
    
    typename std::vector<T>::iterator begin() { return data.begin(); }
    typename std::vector<T>::iterator end() { return data.end(); }
    typename std::vector<T>::const_iterator begin() const { return data.begin(); }
    typename std::vector<T>::const_iterator end() const { return data.end(); }
    
private:
    void checkBounds(size_t index) const {
        if (index >= data.size()) {
            throw ArrayBoundsException(
                "Index " + std::to_string(index) + " out of bounds for array of size " + 
                std::to_string(data.size()) + ": " + debugInfo
            );
        }
    }
};

// 内存安全管理器
class MemorySafetyManager {
public:
    static MemorySafetyManager& getInstance();
    
    // 指针管理
    void registerPointer(void* ptr, std::shared_ptr<std::atomic<bool>> validity);
    void unregisterPointer(void* ptr);
    void invalidatePointer(void* ptr);
    bool isPointerValid(void* ptr) const;
    
    // 内存区域管理
    void registerMemoryRegion(void* start, size_t size, const std::string& debugInfo = "");
    void unregisterMemoryRegion(void* start);
    bool isMemoryRegionValid(void* ptr) const;
    MemoryRegion* findMemoryRegion(void* ptr) const;
    
    // 数组边界检查
    void checkArrayBounds(void* array, size_t index, size_t size, const std::string& debugInfo = "");
    
    // 悬空指针检测
    void checkDanglingPointer(void* ptr, const std::string& debugInfo = "");
    
    // 内存泄漏检测
    std::vector<MemoryRegion> getLeakedRegions() const;
    
    // 调试和统计
    void printMemoryReport() const;
    size_t getTrackedPointersCount() const;
    size_t getTrackedRegionsCount() const;
    
    // 配置选项
    void setStrictMode(bool enabled) { strictMode = enabled; }
    bool isStrictMode() const { return strictMode; }
    
    void setDebugMode(bool enabled) { debugMode = enabled; }
    bool isDebugMode() const { return debugMode; }
    
public:
    ~MemorySafetyManager() = default;
    
private:
    MemorySafetyManager() = default;
    MemorySafetyManager(const MemorySafetyManager&) = delete;
    MemorySafetyManager& operator=(const MemorySafetyManager&) = delete;
    
    mutable std::mutex safetyMutex;
    
    // 指针有效性跟踪
    std::unordered_map<void*, std::weak_ptr<std::atomic<bool>>> trackedPointers;
    
    // 内存区域跟踪
    std::unordered_map<void*, MemoryRegion> memoryRegions;
    
    // 配置选项
    std::atomic<bool> strictMode{true};
    std::atomic<bool> debugMode{false};
    
    static std::unique_ptr<MemorySafetyManager> instance;
    static std::once_flag initFlag;
};

// 便利函数
template<typename T>
SafePtr<T> makeSafe(T* ptr, const std::string& debugInfo = "") {
    return SafePtr<T>(ptr, debugInfo);
}

template<typename T, typename... Args>
SafePtr<T> makeSafeNew(const std::string& debugInfo = "", Args&&... args) {
    T* ptr = new T(std::forward<Args>(args)...);
    MemorySafetyManager::getInstance().registerMemoryRegion(ptr, sizeof(T), debugInfo);
    return SafePtr<T>(ptr, debugInfo);
}

template<typename T>
void safeDelete(SafePtr<T>& ptr) {
    if (ptr) {
        T* rawPtr = ptr.get();
        MemorySafetyManager::getInstance().unregisterMemoryRegion(rawPtr);
        MemorySafetyManager::getInstance().invalidatePointer(rawPtr);
        delete rawPtr;
        ptr.reset();
    }
}

// 类型别名
using SafeValuePtr = SafePtr<Value>;
using SafeValueArray = SafeArray<Value>;

} // namespace miniswift