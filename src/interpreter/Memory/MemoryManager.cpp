#include "MemoryManager.h"
#include "../Value.h"

namespace miniswift {

// ManagedValue实现
size_t ManagedValue::getObjectSize() const {
    return sizeof(Value);
}

} // namespace miniswift
#include <iostream>
#include <thread>
#include <chrono>

namespace miniswift {

// 静态成员初始化
std::unique_ptr<MemoryManager> MemoryManager::instance = nullptr;
std::once_flag MemoryManager::initFlag;

// 获取单例实例
MemoryManager& MemoryManager::getInstance() {
    std::call_once(initFlag, []() {
        instance = std::unique_ptr<MemoryManager>(new MemoryManager());
    });
    return *instance;
}

// 设置配置
void MemoryManager::setConfig(const MemoryConfig& newConfig) {
    std::lock_guard<std::mutex> lock(managerMutex);
    config = newConfig;
    
    // 应用配置到子系统
    MemorySafetyManager::getInstance().setStrictMode(
        config.strategy == MemoryStrategy::STRICT_SAFETY
    );
    MemorySafetyManager::getInstance().setDebugMode(config.enableDebugLogging);
    
    if (config.enableDebugLogging) {
        std::cout << "[MemoryManager] Configuration updated:" << std::endl;
        std::cout << "  Strategy: " << static_cast<int>(config.strategy) << std::endl;
        std::cout << "  Cycle detection: " << (config.enableCycleDetection ? "enabled" : "disabled") << std::endl;
        std::cout << "  Dangling pointer detection: " << (config.enableDanglingPointerDetection ? "enabled" : "disabled") << std::endl;
        std::cout << "  Array bounds checking: " << (config.enableArrayBoundsChecking ? "enabled" : "disabled") << std::endl;
        std::cout << "  Memory leak detection: " << (config.enableMemoryLeakDetection ? "enabled" : "disabled") << std::endl;
    }
}

// 创建托管值
std::shared_ptr<ManagedValue> MemoryManager::createManagedValue(std::unique_ptr<Value> value, const std::string& debugInfo) {
    std::lock_guard<std::mutex> lock(managerMutex);
    
    auto managedValue = std::make_shared<ManagedValue>(std::move(value), debugInfo);
    void* ptr = managedValue->getObjectPointer();
    
    // 注册到托管对象列表
    registerManagedObject(ptr, managedValue);
    
    // 根据配置注册到相应的子系统
    if (config.strategy == MemoryStrategy::ARC_ONLY || 
        config.strategy == MemoryStrategy::ARC_WITH_SAFETY) {
        ARCManager::getInstance().retain(ptr);
    }
    
    if (config.strategy == MemoryStrategy::ARC_WITH_SAFETY || 
        config.strategy == MemoryStrategy::SAFETY_ONLY ||
        config.strategy == MemoryStrategy::STRICT_SAFETY) {
        MemorySafetyManager::getInstance().registerMemoryRegion(
            ptr, managedValue->getObjectSize(), debugInfo
        );
    }
    
    return managedValue;
}

// 创建安全值指针
SafeValuePtr MemoryManager::createSafeValue(std::unique_ptr<Value> value, const std::string& debugInfo) {
    Value* rawPtr = value.release();
    
    // 注册到内存安全管理器
    if (config.strategy == MemoryStrategy::ARC_WITH_SAFETY || 
        config.strategy == MemoryStrategy::SAFETY_ONLY ||
        config.strategy == MemoryStrategy::STRICT_SAFETY) {
        MemorySafetyManager::getInstance().registerMemoryRegion(
            rawPtr, sizeof(Value), debugInfo
        );
    }
    
    return SafeValuePtr(rawPtr, debugInfo);
}

// 内存安全检查
void MemoryManager::checkMemorySafety(void* ptr, const std::string& operation) {
    if (!config.enableDanglingPointerDetection) return;
    
    try {
        MemorySafetyManager::getInstance().checkDanglingPointer(ptr, operation);
    } catch (const MemorySafetyException& e) {
        if (config.enableDebugLogging) {
            std::cerr << "[MemoryManager] Memory safety violation: " << e.what() << std::endl;
        }
        throw;
    }
}

// 数组访问检查
void MemoryManager::checkArrayAccess(void* array, size_t index, size_t size, const std::string& debugInfo) {
    if (!config.enableArrayBoundsChecking) return;
    
    try {
        MemorySafetyManager::getInstance().checkArrayBounds(array, index, size, debugInfo);
    } catch (const MemorySafetyException& e) {
        if (config.enableDebugLogging) {
            std::cerr << "[MemoryManager] Array bounds violation: " << e.what() << std::endl;
        }
        throw;
    }
}

// 检测循环
bool MemoryManager::detectCycles() {
    if (!config.enableCycleDetection) return false;
    
    auto cycles = ARCManager::getInstance().getAllCycles();
    
    if (!cycles.empty() && config.enableDebugLogging) {
        std::cout << "[MemoryManager] Detected " << cycles.size() << " reference cycles" << std::endl;
        for (size_t i = 0; i < cycles.size(); ++i) {
            std::cout << "  Cycle " << (i + 1) << ": ";
            for (void* obj : cycles[i]) {
                std::cout << obj << " -> ";
            }
            std::cout << cycles[i][0] << std::endl;
            
            // 通知循环检测回调
            notifyCycleDetected(cycles[i]);
        }
    }
    
    return !cycles.empty();
}

// 获取所有循环
std::vector<std::vector<void*>> MemoryManager::getAllCycles() {
    if (!config.enableCycleDetection) {
        return {};
    }
    return ARCManager::getInstance().getAllCycles();
}

// 调度循环检测
void MemoryManager::scheduleCycleDetection() {
    if (!config.enableCycleDetection) return;
    
    // 在后台线程中定期检测循环
    std::thread([this]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(config.cycleDetectionInterval));
            
            try {
                detectCycles();
            } catch (const std::exception& e) {
                if (config.enableDebugLogging) {
                    std::cerr << "[MemoryManager] Error in cycle detection: " << e.what() << std::endl;
                }
            }
        }
    }).detach();
}

// 获取内存统计
MemoryManager::MemoryStats MemoryManager::getMemoryStats() const {
    std::lock_guard<std::mutex> lock(managerMutex);
    
    MemoryStats stats;
    stats.totalManagedObjects = managedObjects.size();
    
    // 从ARC管理器获取统计
    auto arcStats = ARCManager::getInstance().getMemoryStats();
    stats.totalStrongRefs = arcStats.totalStrongRefs;
    stats.totalWeakRefs = arcStats.totalWeakRefs;
    stats.totalUnownedRefs = arcStats.totalUnownedRefs;
    stats.detectedCycles = arcStats.detectedCycles;
    
    // 从内存安全管理器获取统计
    stats.trackedPointers = MemorySafetyManager::getInstance().getTrackedPointersCount();
    stats.memoryRegions = MemorySafetyManager::getInstance().getTrackedRegionsCount();
    
    // 检测潜在泄漏
    if (config.enableMemoryLeakDetection) {
        auto leakedRegions = MemorySafetyManager::getInstance().getLeakedRegions();
        stats.potentialLeaks = leakedRegions.size();
    } else {
        stats.potentialLeaks = 0;
    }
    
    return stats;
}

// 打印内存报告
void MemoryManager::printMemoryReport() const {
    auto stats = getMemoryStats();
    
    std::cout << "\n=== Memory Manager Report ===" << std::endl;
    std::cout << "Strategy: ";
    switch (config.strategy) {
        case MemoryStrategy::ARC_ONLY:
            std::cout << "ARC Only";
            break;
        case MemoryStrategy::SAFETY_ONLY:
            std::cout << "Safety Only";
            break;
        case MemoryStrategy::ARC_WITH_SAFETY:
            std::cout << "ARC with Safety";
            break;
        case MemoryStrategy::STRICT_SAFETY:
            std::cout << "Strict Safety";
            break;
    }
    std::cout << std::endl;
    
    std::cout << "Managed objects: " << stats.totalManagedObjects << std::endl;
    std::cout << "Strong references: " << stats.totalStrongRefs << std::endl;
    std::cout << "Weak references: " << stats.totalWeakRefs << std::endl;
    std::cout << "Unowned references: " << stats.totalUnownedRefs << std::endl;
    std::cout << "Detected cycles: " << stats.detectedCycles << std::endl;
    std::cout << "Tracked pointers: " << stats.trackedPointers << std::endl;
    std::cout << "Memory regions: " << stats.memoryRegions << std::endl;
    std::cout << "Potential leaks: " << stats.potentialLeaks << std::endl;
    
    std::cout << "\nFeatures enabled:" << std::endl;
    std::cout << "  Cycle detection: " << (config.enableCycleDetection ? "yes" : "no") << std::endl;
    std::cout << "  Dangling pointer detection: " << (config.enableDanglingPointerDetection ? "yes" : "no") << std::endl;
    std::cout << "  Array bounds checking: " << (config.enableArrayBoundsChecking ? "yes" : "no") << std::endl;
    std::cout << "  Memory leak detection: " << (config.enableMemoryLeakDetection ? "yes" : "no") << std::endl;

    
    std::cout << "============================\n" << std::endl;
}

// 打印详细报告
void MemoryManager::printDetailedReport() const {
    printMemoryReport();
    
    if (config.strategy == MemoryStrategy::ARC_ONLY || 
        config.strategy == MemoryStrategy::ARC_WITH_SAFETY) {
        ARCManager::getInstance().printDebugInfo();
    }
    
    if (config.strategy == MemoryStrategy::ARC_WITH_SAFETY || 
        config.strategy == MemoryStrategy::SAFETY_ONLY ||
        config.strategy == MemoryStrategy::STRICT_SAFETY) {
        MemorySafetyManager::getInstance().printMemoryReport();
    }
}

// 垃圾回收
void MemoryManager::collectGarbage() {
    std::lock_guard<std::mutex> lock(managerMutex);
    
    if (config.enableDebugLogging) {
        std::cout << "[MemoryManager] Starting garbage collection..." << std::endl;
    }
    
    // 检测并处理循环引用
    if (config.enableCycleDetection) {
        auto cycles = ARCManager::getInstance().getAllCycles();
        for (const auto& cycle : cycles) {
            if (config.enableDebugLogging) {
                std::cout << "[MemoryManager] Breaking reference cycle involving " 
                          << cycle.size() << " objects" << std::endl;
            }
            
            // 通知循环检测回调
            notifyCycleDetected(cycle);
            
            // 这里可以实现循环打破逻辑
            // 例如，将循环中的某些强引用转换为弱引用
        }
    }
    
    // 检测内存泄漏
    if (config.enableMemoryLeakDetection) {
        auto leakedRegions = MemorySafetyManager::getInstance().getLeakedRegions();
        for (const auto& region : leakedRegions) {
            if (config.enableDebugLogging) {
                std::cout << "[MemoryManager] Potential memory leak detected: " 
                          << region.start << " (size: " << region.size << ")" << std::endl;
            }
            
            // 通知内存泄漏回调
            notifyMemoryLeak(region.start, region.size);
        }
    }
    
    if (config.enableDebugLogging) {
        std::cout << "[MemoryManager] Garbage collection completed." << std::endl;
    }
}

// 强制清理
void MemoryManager::forceCleanup() {
    std::lock_guard<std::mutex> lock(managerMutex);
    
    if (config.enableDebugLogging) {
        std::cout << "[MemoryManager] Force cleanup initiated..." << std::endl;
    }
    
    // 清理所有托管对象
    for (auto& pair : managedObjects) {
        notifyObjectDestroyed(pair.first);
    }
    managedObjects.clear();
    
    if (config.enableDebugLogging) {
        std::cout << "[MemoryManager] Force cleanup completed." << std::endl;
    }
}

// 设置回调函数
void MemoryManager::setObjectDestroyedCallback(ObjectDestroyedCallback callback) {
    std::lock_guard<std::mutex> lock(managerMutex);
    objectDestroyedCallback = std::move(callback);
}

void MemoryManager::setCycleDetectedCallback(CycleDetectedCallback callback) {
    std::lock_guard<std::mutex> lock(managerMutex);
    cycleDetectedCallback = std::move(callback);
}

void MemoryManager::setMemoryLeakCallback(MemoryLeakCallback callback) {
    std::lock_guard<std::mutex> lock(managerMutex);
    memoryLeakCallback = std::move(callback);
}

// 内部辅助方法
void MemoryManager::registerManagedObject(void* ptr, std::shared_ptr<ManagedObject> obj) {
    managedObjects[ptr] = std::move(obj);
    
    if (config.enableDebugLogging) {
        std::cout << "[MemoryManager] Registered managed object: " << ptr << std::endl;
    }
}

void MemoryManager::unregisterManagedObject(void* ptr) {
    managedObjects.erase(ptr);
    
    if (config.enableDebugLogging) {
        std::cout << "[MemoryManager] Unregistered managed object: " << ptr << std::endl;
    }
}

void MemoryManager::notifyObjectDestroyed(void* ptr) {
    if (objectDestroyedCallback) {
        try {
            objectDestroyedCallback(ptr);
        } catch (const std::exception& e) {
            if (config.enableDebugLogging) {
                std::cerr << "[MemoryManager] Exception in object destroyed callback: " 
                          << e.what() << std::endl;
            }
        }
    }
}

void MemoryManager::notifyCycleDetected(const std::vector<void*>& cycle) {
    if (cycleDetectedCallback) {
        try {
            cycleDetectedCallback(cycle);
        } catch (const std::exception& e) {
            if (config.enableDebugLogging) {
                std::cerr << "[MemoryManager] Exception in cycle detected callback: " 
                          << e.what() << std::endl;
            }
        }
    }
}

void MemoryManager::notifyMemoryLeak(void* ptr, size_t size) {
    if (memoryLeakCallback) {
        try {
            memoryLeakCallback(ptr, size);
        } catch (const std::exception& e) {
            if (config.enableDebugLogging) {
                std::cerr << "[MemoryManager] Exception in memory leak callback: " 
                          << e.what() << std::endl;
            }
        }
    }
}

} // namespace miniswift