#include "MemoryManager.h"
#include "../Value.h"
#include <iostream>
#include <vector>

namespace miniswift {

// 示例：演示ARC和内存安全功能的使用
void demonstrateMemoryManagement() {
    std::cout << "\n=== Memory Management Demonstration ===\n" << std::endl;
    
    // 配置内存管理器
    MemoryConfig config;
    config.strategy = MemoryStrategy::ARC_WITH_SAFETY;
    config.enableCycleDetection = true;
    config.enableDanglingPointerDetection = true;
    config.enableArrayBoundsChecking = true;
    config.enableMemoryLeakDetection = true;
    config.enableDebugLogging = true;
    
    MemoryManager::getInstance().setConfig(config);
    
    std::cout << "1. Creating managed objects with ARC..." << std::endl;
    
    // 创建一些托管对象
    auto value1 = createManaged<Value>("value1");
    auto value2 = createManaged<Value>("value2");
    auto value3 = createManaged<Value>("value3");
    
    std::cout << "\n2. Creating weak and unowned references..." << std::endl;
    
    // 创建弱引用和无主引用
    auto weakRef1 = MemoryManager::getInstance().createWeakRef(value1);
    auto unownedRef2 = MemoryManager::getInstance().createUnownedRef(value2);
    
    std::cout << "\n3. Demonstrating safe array operations..." << std::endl;
    
    // 创建安全数组
    auto safeArray = MemoryManager::getInstance().createSafeArray<int>(5, "demo_array");
    
    // 安全的数组访问
    try {
        safeArray[0] = 10;
        safeArray[4] = 50;
        std::cout << "Array[0] = " << safeArray[0] << ", Array[4] = " << safeArray[4] << std::endl;
        
        // 这将触发边界检查异常
        // safeArray[10] = 100; // 取消注释以测试边界检查
        
    } catch (const ArrayBoundsException& e) {
        std::cout << "Caught array bounds exception: " << e.what() << std::endl;
    }
    
    std::cout << "\n4. Creating reference cycles for detection..." << std::endl;
    
    // 模拟创建循环引用（在实际使用中应该避免）
    // 这里我们手动添加强引用关系来演示循环检测
    ARCManager::getInstance().addStrongReference(value1.get(), value2.get());
    ARCManager::getInstance().addStrongReference(value2.get(), value3.get());
    ARCManager::getInstance().addStrongReference(value3.get(), value1.get());
    
    std::cout << "\n5. Detecting reference cycles..." << std::endl;
    
    bool hasCycles = MemoryManager::getInstance().detectCycles();
    if (hasCycles) {
        std::cout << "Reference cycles detected!" << std::endl;
        auto cycles = MemoryManager::getInstance().getAllCycles();
        std::cout << "Number of cycles: " << cycles.size() << std::endl;
    }
    
    std::cout << "\n6. Testing weak reference behavior..." << std::endl;
    
    // 测试弱引用
    if (auto strongFromWeak = weakRef1.lock()) {
        std::cout << "Weak reference is still valid" << std::endl;
    } else {
        std::cout << "Weak reference has been invalidated" << std::endl;
    }
    
    std::cout << "\n7. Memory statistics before cleanup..." << std::endl;
    MemoryManager::getInstance().printMemoryReport();
    
    std::cout << "\n8. Performing garbage collection..." << std::endl;
    MemoryManager::getInstance().collectGarbage();
    
    std::cout << "\n9. Cleaning up objects..." << std::endl;
    
    // 清理循环引用
    ARCManager::getInstance().removeStrongReference(value1.get(), value2.get());
    ARCManager::getInstance().removeStrongReference(value2.get(), value3.get());
    ARCManager::getInstance().removeStrongReference(value3.get(), value1.get());
    
    // 销毁对象
    destroyManaged(value1);
    destroyManaged(value2);
    destroyManaged(value3);
    
    std::cout << "\n10. Final memory statistics..." << std::endl;
    MemoryManager::getInstance().printMemoryReport();
    
    std::cout << "\n=== Demonstration Complete ===\n" << std::endl;
}

// 示例：演示内存安全检查
void demonstrateMemorySafety() {
    std::cout << "\n=== Memory Safety Demonstration ===\n" << std::endl;
    
    // 配置为严格安全模式
    MemoryConfig config;
    config.strategy = MemoryStrategy::STRICT_SAFETY;
    config.enableDebugLogging = true;
    MemoryManager::getInstance().setConfig(config);
    
    std::cout << "1. Creating safe pointers..." << std::endl;
    
    // 创建安全指针
    auto safePtr = makeSafe(new int(42), "safe_int");
    std::cout << "Safe pointer value: " << *safePtr << std::endl;
    
    std::cout << "\n2. Testing dangling pointer detection..." << std::endl;
    
    try {
        // 创建一个指针并立即删除，然后尝试访问
        int* rawPtr = new int(100);
        auto safePtr2 = makeSafe(rawPtr, "temp_int");
        
        // 模拟删除
        MemorySafetyManager::getInstance().invalidatePointer(rawPtr);
        
        // 这应该触发悬空指针异常
        // std::cout << "Value: " << *safePtr2 << std::endl; // 取消注释以测试
        
    } catch (const DanglingPointerException& e) {
        std::cout << "Caught dangling pointer exception: " << e.what() << std::endl;
    }
    
    std::cout << "\n3. Testing array bounds checking..." << std::endl;
    
    try {
        auto safeArray = MemoryManager::getInstance().createSafeArray<std::string>(3, "string_array");
        safeArray[0] = "Hello";
        safeArray[1] = "World";
        safeArray[2] = "!";
        
        std::cout << "Array contents: ";
        for (size_t i = 0; i < safeArray.size(); ++i) {
            std::cout << safeArray[i] << " ";
        }
        std::cout << std::endl;
        
        // 这将触发边界检查异常
        // safeArray[5] = "OutOfBounds"; // 取消注释以测试
        
    } catch (const ArrayBoundsException& e) {
        std::cout << "Caught array bounds exception: " << e.what() << std::endl;
    }
    
    std::cout << "\n4. Memory safety report..." << std::endl;
    MemorySafetyManager::getInstance().printMemoryReport();
    
    // 清理
    safeDelete(safePtr);
    
    std::cout << "\n=== Memory Safety Demonstration Complete ===\n" << std::endl;
}

// 示例：演示不同内存管理策略
void demonstrateMemoryStrategies() {
    std::cout << "\n=== Memory Strategy Comparison ===\n" << std::endl;
    
    std::vector<MemoryStrategy> strategies = {
        MemoryStrategy::ARC_ONLY,
        MemoryStrategy::SAFETY_ONLY,
        MemoryStrategy::ARC_WITH_SAFETY,
        MemoryStrategy::STRICT_SAFETY
    };
    
    std::vector<std::string> strategyNames = {
        "ARC Only",
        "Safety Only", 
        "ARC with Safety",
        "Strict Safety"
    };
    
    for (size_t i = 0; i < strategies.size(); ++i) {
        std::cout << "\nTesting strategy: " << strategyNames[i] << std::endl;
        
        MemoryConfig config;
        config.strategy = strategies[i];
        config.enableDebugLogging = false; // 减少输出
        MemoryManager::getInstance().setConfig(config);
        
        // 创建一些对象
        auto obj1 = createManaged<Value>("test_obj1");
        auto obj2 = createManaged<Value>("test_obj2");
        
        // 获取统计信息
        auto stats = MemoryManager::getInstance().getMemoryStats();
        std::cout << "  Managed objects: " << stats.totalManagedObjects << std::endl;
        std::cout << "  Tracked pointers: " << stats.trackedPointers << std::endl;
        std::cout << "  Memory regions: " << stats.memoryRegions << std::endl;
        
        // 清理
        destroyManaged(obj1);
        destroyManaged(obj2);
    }
    
    std::cout << "\n=== Strategy Comparison Complete ===\n" << std::endl;
}

} // namespace miniswift

// 主函数用于测试
int main() {
    using namespace miniswift;
    
    try {
        demonstrateMemoryManagement();
        demonstrateMemorySafety();
        demonstrateMemoryStrategies();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}