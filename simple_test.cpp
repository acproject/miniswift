#include "src/interpreter/Memory/MemoryManager.h"
#include "src/interpreter/Value.h"
#include <iostream>

using namespace miniswift;

int main() {
    std::cout << "=== 简化内存管理测试 ===\n\n";
    
    try {
        // 获取内存管理器实例
        auto& manager = MemoryManager::getInstance();
        
        // 配置内存管理策略
        MemoryConfig config;
        config.strategy = MemoryStrategy::ARC_WITH_SAFETY;
        config.enableCycleDetection = true;
        config.enableMemoryLeakDetection = true;
        config.enableArrayBoundsChecking = true;
        config.enableDanglingPointerDetection = true;
        config.enableDebugLogging = false;  // 关闭调试日志
        manager.setConfig(config);
        
        std::cout << "1. 内存管理器配置完成\n";
        
        // 创建一些测试值
        Value testValue1(42);
        Value testValue2(std::string("Hello, MiniSwift!"));
        
        // 创建托管值
        auto testValuePtr1 = std::make_unique<Value>(testValue1);
        auto testValuePtr2 = std::make_unique<Value>(testValue2);
        auto managedValue1 = manager.createManagedValue(std::move(testValuePtr1), "test_value_1");
        auto managedValue2 = manager.createManagedValue(std::move(testValuePtr2), "test_value_2");
        
        std::cout << "2. 创建托管值完成\n";
        
        // 创建安全数组
        auto safeArray = manager.createSafeArray<int>(5, "test_array");
        safeArray[0] = 10;
        safeArray[1] = 20;
        safeArray[2] = 30;
        
        std::cout << "3. 创建安全数组完成\n";
        
        // 获取内存统计（不调用printMemoryReport）
        auto stats = manager.getMemoryStats();
        std::cout << "4. 内存统计:\n";
        std::cout << "   - 托管对象数量: " << stats.totalManagedObjects << "\n";
        std::cout << "   - 强引用数量: " << stats.totalStrongRefs << "\n";
        std::cout << "   - 弱引用数量: " << stats.totalWeakRefs << "\n";
        std::cout << "   - 无主引用数量: " << stats.totalUnownedRefs << "\n";
        std::cout << "   - 跟踪指针数量: " << stats.trackedPointers << "\n";
        std::cout << "   - 检测到的循环: " << stats.detectedCycles << "\n";
        
        std::cout << "\n=== 测试完成 ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}