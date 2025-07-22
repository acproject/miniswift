#pragma once

#include "ARC.h"
#include "MemorySafety.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>

namespace miniswift {

// 前向声明
struct Value;
struct ClassInstance;

// 内存管理策略
enum class MemoryStrategy {
    ARC_ONLY,           // 仅使用ARC
    SAFETY_ONLY,        // 仅使用内存安全检查
    ARC_WITH_SAFETY,    // ARC + 内存安全检查
    STRICT_SAFETY       // 严格的内存安全检查
};

// 内存管理配置
struct MemoryConfig {
    MemoryStrategy strategy = MemoryStrategy::ARC_WITH_SAFETY;
    bool enableCycleDetection = true;
    bool enableDanglingPointerDetection = true;
    bool enableArrayBoundsChecking = true;
    bool enableMemoryLeakDetection = true;
    bool enableDebugLogging = false;
    size_t cycleDetectionInterval = 1000; // 毫秒
    size_t maxCycleDepth = 100;
};

// 托管对象基类
class ManagedObject {
public:
    virtual ~ManagedObject() = default;
    virtual void* getObjectPointer() = 0;
    virtual size_t getObjectSize() const = 0;
    virtual std::string getDebugInfo() const = 0;
};

// 托管值包装器
class ManagedValue : public ManagedObject {
private:
    std::unique_ptr<Value> value;
    std::string debugInfo;
    
public:
    explicit ManagedValue(std::unique_ptr<Value> val, const std::string& info = "")
        : value(std::move(val)), debugInfo(info) {}
    
    Value* getValue() const { return value.get(); }
    void* getObjectPointer() override { return value.get(); }
    size_t getObjectSize() const override;
    std::string getDebugInfo() const override { return debugInfo; }
    
    // 禁止拷贝，只允许移动
    ManagedValue(const ManagedValue&) = delete;
    ManagedValue& operator=(const ManagedValue&) = delete;
    ManagedValue(ManagedValue&&) = default;
    ManagedValue& operator=(ManagedValue&&) = default;
};

// 统一的内存管理器
class MemoryManager {
public:
    static MemoryManager& getInstance();
    
    // 配置管理
    void setConfig(const MemoryConfig& config);
    const MemoryConfig& getConfig() const { return config; }
    
    // 对象生命周期管理
    template<typename T, typename... Args>
    ARCPtr<T> createObject(const std::string& debugInfo = "", Args&&... args);
    
    template<typename T>
    void destroyObject(ARCPtr<T>& ptr);
    
    // Value 特化管理
    std::shared_ptr<ManagedValue> createManagedValue(std::unique_ptr<Value> value, const std::string& debugInfo = "");
    SafeValuePtr createSafeValue(std::unique_ptr<Value> value, const std::string& debugInfo = "");
    
    // 引用管理
    template<typename T>
    WeakRef<T> createWeakRef(const ARCPtr<T>& strongRef);
    
    template<typename T>
    UnownedRef<T> createUnownedRef(const ARCPtr<T>& strongRef);
    
    // 数组管理
    template<typename T>
    SafeArray<T> createSafeArray(size_t size, const std::string& debugInfo = "");
    
    template<typename T>
    SafeArray<T> createSafeArray(std::initializer_list<T> init, const std::string& debugInfo = "");
    
    // 内存安全检查
    void checkMemorySafety(void* ptr, const std::string& operation = "");
    void checkArrayAccess(void* array, size_t index, size_t size, const std::string& debugInfo = "");
    
    // 循环检测
    bool detectCycles();
    std::vector<std::vector<void*>> getAllCycles();
    void scheduleCycleDetection();
    
    // 内存统计和调试
    struct MemoryStats {
        size_t totalManagedObjects;
        size_t totalStrongRefs;
        size_t totalWeakRefs;
        size_t totalUnownedRefs;
        size_t detectedCycles;
        size_t trackedPointers;
        size_t memoryRegions;
        size_t potentialLeaks;
    };
    
    MemoryStats getMemoryStats() const;
    void printMemoryReport() const;
    void printDetailedReport() const;
    
    // 垃圾回收
    void collectGarbage();
    void forceCleanup();
    
    // 事件回调
    using ObjectDestroyedCallback = std::function<void(void*)>;
    using CycleDetectedCallback = std::function<void(const std::vector<void*>&)>;
    using MemoryLeakCallback = std::function<void(void*, size_t)>;
    
    void setObjectDestroyedCallback(ObjectDestroyedCallback callback);
    void setCycleDetectedCallback(CycleDetectedCallback callback);
    void setMemoryLeakCallback(MemoryLeakCallback callback);
    
public:
    ~MemoryManager() = default;
    
private:
    MemoryManager() = default;
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
    
    MemoryConfig config;
    mutable std::mutex managerMutex;
    
    // 托管对象跟踪
    std::unordered_map<void*, std::shared_ptr<ManagedObject>> managedObjects;
    
    // 事件回调
    ObjectDestroyedCallback objectDestroyedCallback;
    CycleDetectedCallback cycleDetectedCallback;
    MemoryLeakCallback memoryLeakCallback;
    
    // 内部辅助方法
    void registerManagedObject(void* ptr, std::shared_ptr<ManagedObject> obj);
    void unregisterManagedObject(void* ptr);
    void notifyObjectDestroyed(void* ptr);
    void notifyCycleDetected(const std::vector<void*>& cycle);
    void notifyMemoryLeak(void* ptr, size_t size);
    
    static std::unique_ptr<MemoryManager> instance;
    static std::once_flag initFlag;
};

// 模板实现
template<typename T, typename... Args>
ARCPtr<T> MemoryManager::createObject(const std::string& debugInfo, Args&&... args) {
    std::lock_guard<std::mutex> lock(managerMutex);
    
    // 创建对象
    auto obj = std::make_unique<T>(std::forward<Args>(args)...);
    T* rawPtr = obj.get();
    
    // 根据配置选择内存管理策略
    ARCPtr<T> arcPtr;
    
    switch (config.strategy) {
        case MemoryStrategy::ARC_ONLY:
        case MemoryStrategy::ARC_WITH_SAFETY:
            arcPtr = makeARC<T>(obj.release(), debugInfo);
            break;
            
        case MemoryStrategy::SAFETY_ONLY:
        case MemoryStrategy::STRICT_SAFETY:
            // 即使不使用ARC，也要创建ARCPtr来保持接口一致性
            arcPtr = makeARC<T>(obj.release(), debugInfo);
            break;
    }
    
    // 注册到内存安全管理器
    if (config.strategy == MemoryStrategy::ARC_WITH_SAFETY || 
        config.strategy == MemoryStrategy::SAFETY_ONLY ||
        config.strategy == MemoryStrategy::STRICT_SAFETY) {
        
        MemorySafetyManager::getInstance().registerMemoryRegion(
            rawPtr, sizeof(T), debugInfo
        );
        
        if (config.strategy == MemoryStrategy::STRICT_SAFETY) {
            MemorySafetyManager::getInstance().setStrictMode(true);
        }
    }
    
    return arcPtr;
}

template<typename T>
void MemoryManager::destroyObject(ARCPtr<T>& ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(managerMutex);
    
    T* rawPtr = ptr.get();
    
    // 通知对象即将被销毁
    notifyObjectDestroyed(rawPtr);
    
    // 从内存安全管理器中注销
    if (config.strategy == MemoryStrategy::ARC_WITH_SAFETY || 
        config.strategy == MemoryStrategy::SAFETY_ONLY ||
        config.strategy == MemoryStrategy::STRICT_SAFETY) {
        
        MemorySafetyManager::getInstance().unregisterMemoryRegion(rawPtr);
        MemorySafetyManager::getInstance().invalidatePointer(rawPtr);
    }
    
    // 重置ARCPtr，这将触发ARC的清理逻辑
    ptr.reset();
}

template<typename T>
WeakRef<T> MemoryManager::createWeakRef(const ARCPtr<T>& strongRef) {
    return WeakRef<T>(strongRef);
}

template<typename T>
UnownedRef<T> MemoryManager::createUnownedRef(const ARCPtr<T>& strongRef) {
    return UnownedRef<T>(strongRef);
}

template<typename T>
SafeArray<T> MemoryManager::createSafeArray(size_t size, const std::string& debugInfo) {
    return SafeArray<T>(size, debugInfo);
}

template<typename T>
SafeArray<T> MemoryManager::createSafeArray(std::initializer_list<T> init, const std::string& debugInfo) {
    return SafeArray<T>(init, debugInfo);
}

// 便利函数
template<typename T, typename... Args>
ARCPtr<T> createManaged(const std::string& debugInfo = "", Args&&... args) {
    return MemoryManager::getInstance().createObject<T>(debugInfo, std::forward<Args>(args)...);
}

template<typename T>
void destroyManaged(ARCPtr<T>& ptr) {
    MemoryManager::getInstance().destroyObject(ptr);
}

} // namespace miniswift