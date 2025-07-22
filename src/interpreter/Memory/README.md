# MiniSwift 内存管理模块

本模块实现了完整的自动引用计数（ARC）和内存安全检查功能，为 MiniSwift 解释器提供了强大的内存管理能力。

## 功能特性

### 1. 自动引用计数（ARC）
- **强引用管理**：自动跟踪对象的引用计数
- **弱引用（weak）**：避免循环引用，当对象被释放时自动置为 nil
- **无主引用（unowned）**：不增加引用计数，但假设对象在引用期间始终存在
- **循环引用检测**：自动检测和报告强引用循环
- **线程安全**：所有操作都是线程安全的

### 2. 内存安全检查
- **悬空指针检测**：防止访问已释放的内存
- **数组边界检查**：防止数组越界访问
- **内存泄漏检测**：识别潜在的内存泄漏
- **使用后释放检测**：防止使用已释放的对象

### 3. 统一内存管理
- **多种策略**：支持不同的内存管理策略
- **配置灵活**：可以根据需要启用或禁用特定功能
- **性能监控**：提供详细的内存使用统计
- **调试支持**：丰富的调试信息和报告

## 核心组件

### ARC.h/ARC.cpp
实现自动引用计数的核心功能：
- `ARCManager`：管理引用计数和循环检测
- `ARCPtr<T>`：智能指针包装器
- `WeakRef<T>`：弱引用实现
- `UnownedRef<T>`：无主引用实现

### MemorySafety.h/MemorySafety.cpp
实现内存安全检查功能：
- `MemorySafetyManager`：管理内存安全检查
- `SafePtr<T>`：安全指针包装器
- `SafeArray<T>`：安全数组包装器
- 各种内存安全异常类

### MemoryManager.h/MemoryManager.cpp
提供统一的内存管理接口：
- `MemoryManager`：统一的内存管理器
- `MemoryConfig`：内存管理配置
- `ManagedObject`：托管对象基类

## 使用方法

### 基本用法

```cpp
#include "Memory/MemoryManager.h"

using namespace miniswift;

// 配置内存管理器
MemoryConfig config;
config.strategy = MemoryStrategy::ARC_WITH_SAFETY;
config.enableCycleDetection = true;
config.enableDanglingPointerDetection = true;
MemoryManager::getInstance().setConfig(config);

// 创建托管对象
auto obj = createManaged<MyClass>("debug_info", constructorArgs...);

// 创建弱引用
auto weakRef = MemoryManager::getInstance().createWeakRef(obj);

// 创建安全数组
auto safeArray = MemoryManager::getInstance().createSafeArray<int>(10, "my_array");

// 安全访问
safeArray[0] = 42;
int value = safeArray[0];

// 清理
destroyManaged(obj);
```

### 内存管理策略

1. **ARC_ONLY**：仅使用自动引用计数
2. **SAFETY_ONLY**：仅使用内存安全检查
3. **ARC_WITH_SAFETY**：结合 ARC 和内存安全检查（推荐）
4. **STRICT_SAFETY**：严格的内存安全检查

### 循环引用处理

```cpp
// 检测循环引用
bool hasCycles = MemoryManager::getInstance().detectCycles();
if (hasCycles) {
    auto cycles = MemoryManager::getInstance().getAllCycles();
    // 处理检测到的循环
}

// 使用弱引用打破循环
class Parent {
    ARCPtr<Child> child;
};

class Child {
    WeakRef<Parent> parent; // 使用弱引用避免循环
};
```

### 内存安全检查

```cpp
// 安全指针
auto safePtr = makeSafe(new MyClass(), "my_object");

// 自动检测悬空指针
try {
    auto value = *safePtr; // 如果对象已被释放，会抛出异常
} catch (const DanglingPointerException& e) {
    std::cout << "Dangling pointer detected: " << e.what() << std::endl;
}

// 数组边界检查
try {
    safeArray[100] = 42; // 如果越界，会抛出异常
} catch (const ArrayBoundsException& e) {
    std::cout << "Array bounds violation: " << e.what() << std::endl;
}
```

### 调试和监控

```cpp
// 获取内存统计
auto stats = MemoryManager::getInstance().getMemoryStats();
std::cout << "Managed objects: " << stats.totalManagedObjects << std::endl;
std::cout << "Detected cycles: " << stats.detectedCycles << std::endl;

// 打印详细报告
MemoryManager::getInstance().printDetailedReport();

// 执行垃圾回收
MemoryManager::getInstance().collectGarbage();
```

### 事件回调

```cpp
// 设置对象销毁回调
MemoryManager::getInstance().setObjectDestroyedCallback(
    [](void* ptr) {
        std::cout << "Object destroyed: " << ptr << std::endl;
    }
);

// 设置循环检测回调
MemoryManager::getInstance().setCycleDetectedCallback(
    [](const std::vector<void*>& cycle) {
        std::cout << "Cycle detected with " << cycle.size() << " objects" << std::endl;
    }
);
```

## 配置选项

```cpp
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
```

## 异常类型

- `MemorySafetyException`：基础内存安全异常
- `DanglingPointerException`：悬空指针异常
- `ArrayBoundsException`：数组边界异常
- `UseAfterFreeException`：使用后释放异常

## 性能考虑

1. **引用计数开销**：每次赋值和拷贝都会更新引用计数
2. **循环检测开销**：定期的循环检测会消耗 CPU 时间
3. **内存安全检查开销**：每次内存访问都会进行安全检查
4. **线程同步开销**：多线程环境下的同步开销

建议在生产环境中根据实际需求调整配置，在开发和调试阶段启用所有安全检查。

## 示例代码

参见 `MemoryExample.cpp` 文件，其中包含了完整的使用示例和演示代码。

## 注意事项

1. **避免循环引用**：尽量使用弱引用或无主引用来打破循环
2. **及时清理**：不再需要的对象应该及时销毁
3. **异常处理**：正确处理内存安全异常
4. **性能监控**：定期检查内存使用情况和性能指标
5. **配置调优**：根据应用场景调整内存管理配置

## 集成到 MiniSwift

要在 MiniSwift 解释器中使用这个内存管理模块：

1. 在 `Value.h` 中包含内存管理头文件
2. 将 `ClassInstance` 等对象改为使用 `ARCPtr` 管理
3. 在数组和字典操作中使用 `SafeArray` 进行边界检查
4. 在解释器初始化时配置内存管理策略
5. 在适当的时机执行垃圾回收和循环检测

这样可以为 MiniSwift 提供与 Swift 语言类似的内存安全保障。