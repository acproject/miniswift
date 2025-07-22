#include "src/interpreter/Memory/MemoryManager.h"
#include "src/interpreter/Value.h"
#include <iostream>

using namespace miniswift;

int main() {
    std::cout << "=== 最小内存管理测试 ===\n\n";
    
    try {
        // 获取内存管理器实例
        auto& manager = MemoryManager::getInstance();
        std::cout << "1. 内存管理器实例获取成功\n";
        
        // 简单配置
        MemoryConfig config;
        config.strategy = MemoryStrategy::ARC_ONLY;  // 只使用ARC，避免复杂的安全检查
        config.enableCycleDetection = false;
        config.enableMemoryLeakDetection = false;
        config.enableArrayBoundsChecking = false;
        config.enableDanglingPointerDetection = false;
        config.enableDebugLogging = false;
        manager.setConfig(config);
        std::cout << "2. 内存管理器配置完成\n";
        
        // 创建简单的测试值
        Value testValue(42);
        std::cout << "3. 创建测试值完成\n";
        
        // 创建托管值
        auto testValuePtr = std::make_unique<Value>(testValue);
        auto managedValue = manager.createManagedValue(std::move(testValuePtr), "test_value");
        std::cout << "4. 创建托管值完成\n";
        
        std::cout << "\n=== 测试完成 ===\n";
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}