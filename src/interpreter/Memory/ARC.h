#ifndef MINISWIFT_ARC_H
#define MINISWIFT_ARC_H

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>
#include <string>
#include <stdexcept>

namespace miniswift {

// Forward declarations
struct Value;
struct ClassInstance;

// Reference types for ARC
enum class ReferenceType {
    Strong,    // 强引用
    Weak,      // 弱引用
    Unowned    // 无主引用
};

// Reference information for tracking
struct ReferenceInfo {
    void* object;           // 指向对象的指针
    ReferenceType type;     // 引用类型
    std::string location;   // 引用位置（用于调试）
    
    ReferenceInfo(void* obj, ReferenceType refType, const std::string& loc = "")
        : object(obj), type(refType), location(loc) {}
};

// Weak reference wrapper
template<typename T>
class WeakRef {
private:
    std::weak_ptr<T> weakPtr;
    mutable std::atomic<bool> isValid{true};
    
public:
    WeakRef() = default;
    
    explicit WeakRef(std::shared_ptr<T> ptr) : weakPtr(ptr) {}
    
    // 检查引用是否仍然有效
    bool isAlive() const {
        return !weakPtr.expired() && isValid.load();
    }
    
    // 获取强引用（如果对象仍然存在）
    std::shared_ptr<T> lock() const {
        if (isValid.load()) {
            return weakPtr.lock();
        }
        return nullptr;
    }
    
    // 重置弱引用
    void reset() {
        weakPtr.reset();
        isValid.store(false);
    }
    
    // 赋值操作
    WeakRef& operator=(std::shared_ptr<T> ptr) {
        weakPtr = ptr;
        isValid.store(true);
        return *this;
    }
    
    bool operator==(const WeakRef& other) const {
        return weakPtr.lock() == other.weakPtr.lock();
    }
    
    bool operator!=(const WeakRef& other) const {
        return !(*this == other);
    }
};

// Unowned reference wrapper
template<typename T>
class UnownedRef {
private:
    T* rawPtr;
    mutable std::atomic<bool> isValid{true};
    
public:
    UnownedRef() : rawPtr(nullptr) {}
    
    explicit UnownedRef(T* ptr) : rawPtr(ptr) {}
    
    explicit UnownedRef(std::shared_ptr<T> ptr) : rawPtr(ptr.get()) {}
    
    // 获取原始指针（危险操作，需要确保对象仍然存在）
    T* get() const {
        if (isValid.load() && rawPtr) {
            return rawPtr;
        }
        throw std::runtime_error("Attempted to access deallocated unowned reference");
    }
    
    // 检查引用是否有效
    bool isAlive() const {
        return isValid.load() && rawPtr != nullptr;
    }
    
    // 标记为无效（当对象被销毁时调用）
    void invalidate() {
        isValid.store(false);
    }
    
    // 重置引用
    void reset() {
        rawPtr = nullptr;
        isValid.store(false);
    }
    
    // 赋值操作
    UnownedRef& operator=(T* ptr) {
        rawPtr = ptr;
        isValid.store(true);
        return *this;
    }
    
    UnownedRef& operator=(std::shared_ptr<T> ptr) {
        rawPtr = ptr.get();
        isValid.store(true);
        return *this;
    }
    
    // 解引用操作符
    T& operator*() const {
        return *get();
    }
    
    T* operator->() const {
        return get();
    }
    
    bool operator==(const UnownedRef& other) const {
        return rawPtr == other.rawPtr;
    }
    
    bool operator!=(const UnownedRef& other) const {
        return !(*this == other);
    }
};

// ARC管理器类
class ARCManager {
private:
    // 对象引用计数映射
    std::unordered_map<void*, int> refCounts;
    
    // 弱引用跟踪
    std::unordered_map<void*, std::vector<std::function<void()>>> weakRefCallbacks;
    
    // 无主引用跟踪
    std::unordered_map<void*, std::vector<std::function<void()>>> unownedRefCallbacks;
    
    // 强引用循环检测
    std::unordered_map<void*, std::unordered_set<void*>> strongReferences;
    
    // 线程安全
    mutable std::mutex arcMutex;
    
    // 单例实例
    static std::unique_ptr<ARCManager> instance;
    static std::once_flag initFlag;
    
public:
    ~ARCManager() = default;
    
private:
    ARCManager() = default;
    ARCManager(const ARCManager&) = delete;
    ARCManager& operator=(const ARCManager&) = delete;
    
public:
    // 获取单例实例
    static ARCManager& getInstance();
    
    // 增加引用计数
    void retain(void* object);
    
    // 减少引用计数
    void release(void* object);
    
    // 获取引用计数
    int getRefCount(void* object) const;
    
    // 注册弱引用
    void registerWeakRef(void* object, std::function<void()> callback);
    
    // 注册无主引用
    void registerUnownedRef(void* object, std::function<void()> callback);
    
    // 添加强引用关系（用于循环检测）
    void addStrongReference(void* from, void* to);
    
    // 移除强引用关系
    void removeStrongReference(void* from, void* to);
    
    // 检测强引用循环
    bool detectCycle(void* startObject) const;
    
    // 获取所有强引用循环
    std::vector<std::vector<void*>> getAllCycles() const;
    
    // 清理对象（当引用计数为0时调用）
    void cleanup(void* object);
    
    // 调试信息
    void printDebugInfo() const;
    
    // 获取内存统计信息
    struct MemoryStats {
        size_t totalObjects;
        size_t totalStrongRefs;
        size_t totalWeakRefs;
        size_t totalUnownedRefs;
        size_t detectedCycles;
    };
    
    MemoryStats getMemoryStats() const;
    
private:
    // 深度优先搜索检测循环
    bool dfsDetectCycle(void* current, void* target, std::unordered_set<void*>& visited, std::unordered_set<void*>& recursionStack) const;
    
    // 查找循环路径的辅助函数
    bool findCyclePath(void* current, 
                      std::unordered_set<void*>& visited,
                      std::unordered_set<void*>& recursionStack,
                      std::vector<void*>& currentPath,
                      std::vector<std::vector<void*>>& cycles) const;
    
    // 通知弱引用对象已被销毁
    void notifyWeakRefs(void* object);
    
    // 通知无主引用对象已被销毁
    void notifyUnownedRefs(void* object);
};

// RAII包装器，用于自动管理ARC
template<typename T>
class ARCPtr {
private:
    std::shared_ptr<T> ptr;
    
public:
    ARCPtr() = default;
    
    explicit ARCPtr(T* rawPtr) : ptr(rawPtr, [](T* p) {
        ARCManager::getInstance().release(p);
        delete p;
    }) {
        if (rawPtr) {
            ARCManager::getInstance().retain(rawPtr);
        }
    }
    
    ARCPtr(const ARCPtr& other) : ptr(other.ptr) {
        if (ptr) {
            ARCManager::getInstance().retain(ptr.get());
        }
    }
    
    ARCPtr(ARCPtr&& other) noexcept : ptr(std::move(other.ptr)) {}
    
    ARCPtr& operator=(const ARCPtr& other) {
        if (this != &other) {
            if (ptr) {
                ARCManager::getInstance().release(ptr.get());
            }
            ptr = other.ptr;
            if (ptr) {
                ARCManager::getInstance().retain(ptr.get());
            }
        }
        return *this;
    }
    
    ARCPtr& operator=(ARCPtr&& other) noexcept {
        if (this != &other) {
            if (ptr) {
                ARCManager::getInstance().release(ptr.get());
            }
            ptr = std::move(other.ptr);
        }
        return *this;
    }
    
    ~ARCPtr() {
        if (ptr) {
            ARCManager::getInstance().release(ptr.get());
        }
    }
    
    T* get() const { return ptr.get(); }
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr.get(); }
    
    explicit operator bool() const { return ptr != nullptr; }
    
    bool operator==(const ARCPtr& other) const { return ptr == other.ptr; }
    bool operator!=(const ARCPtr& other) const { return !(*this == other); }
    
    // 创建弱引用
    WeakRef<T> createWeakRef() const {
        return WeakRef<T>(ptr);
    }
    
    // 创建无主引用
    UnownedRef<T> createUnownedRef() const {
        return UnownedRef<T>(ptr);
    }
};

// 便利函数
template<typename T, typename... Args>
ARCPtr<T> makeARC(Args&&... args) {
    return ARCPtr<T>(new T(std::forward<Args>(args)...));
}

// 类型别名
using WeakClassRef = WeakRef<ClassInstance>;
using UnownedClassRef = UnownedRef<ClassInstance>;
using ARCClassPtr = ARCPtr<ClassInstance>;

} // namespace miniswift

#endif // MINISWIFT_ARC_H