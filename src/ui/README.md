# MiniSwift UI System

MiniSwift UI系统为MiniSwift编程语言提供了现代化的用户界面开发能力，支持声明式UI编程范式。

## 架构概述

### 核心组件

- **UIRuntime**: 核心UI运行时，提供基础UI组件和应用生命周期管理
- **DuorouBackend**: duorou_gui 后端适配（将 UIWidget 树转换为 ViewNode）
- **UIIntegration**: Swift语言与C++ UI运行时的集成桥梁（开发中）

### 支持的UI组件

- **TextWidget**: 文本显示组件
- **ButtonWidget**: 按钮组件，支持点击事件
- **VStackWidget**: 垂直布局容器
- **HStackWidget**: 水平布局容器

### 布局系统

- 基于几何类型（Point, Size, Rect）的精确布局
- 支持边距（EdgeInsets）和对齐方式（Alignment）
- 自动布局计算和渲染

## 快速开始

### 1. 基本使用

```cpp
#include "UIRuntime.h"

using namespace MiniSwift::UI;

int main() {
    // 初始化UI应用
    UIApplication& app = UIApplication::getInstance();
    app.initialize();
    
    // 创建UI组件
    auto text = createText("Hello, MiniSwift!");
    auto button = createButton("Click Me", []() {
        std::cout << "Button clicked!" << std::endl;
    });
    
    // 创建布局
    auto vstack = createVStack(12.0);
    vstack->addChild(text);
    vstack->addChild(button);
    
    // 设置根视图
    app.setRootWidget(vstack);
    
    // 运行应用（在实际应用中）
    // app.run();
    
    return 0;
}
```

### 2. 样式和颜色

```cpp
// 设置文本颜色
text->setTextColor(Color::blueColor());

// 设置背景色
button->setBackgroundColor(Color{0.2, 0.6, 1.0, 1.0});

// 设置边距
button->setPadding(EdgeInsets{10, 20, 10, 20});

// 设置圆角
button->setCornerRadius(8.0);
```

### 3. 布局容器

```cpp
// 垂直布局
auto vstack = createVStack(16.0); // 16点间距
vstack->addChild(header);
vstack->addChild(content);
vstack->addChild(footer);

// 水平布局
auto hstack = createHStack(8.0); // 8点间距
hstack->addChild(leftPanel);
hstack->addChild(rightPanel);
```

## 后端支持

### Duorou后端

在 duorou_gui 项目中构建时，可以启用 duorou 后端，将 MiniSwift 的 `UIWidget` 树转换为 duorou 的 `ViewNode` 树进行渲染。

### Mock后端

当未启用 duorou 后端时，系统会回退到 Mock 后端，提供控制台输出用于调试：

```
[TextWidget] Rendering text: "Hello, World!"
[ButtonWidget] Rendering button: "Click Me"
[VStackWidget] Rendering VStack with 2 children
```

## 编译配置

### CMake集成

UI系统已集成到主项目的CMake配置中：

```cmake
# UI模块会自动包含在构建中
# duorou 后端会在检测到 duorou_gui 头文件时自动启用（可通过 MINISWIFT_UI_ENABLE_DUOROU 控制）
```

### 手动编译

```bash
# 编译UI测试程序
g++ -std=c++20 -I src -I src/ui -o ui_test \
    src/ui/UITest.cpp \
    src/ui/UIRuntime.cpp \
    src/ui/UIIntegration.cpp

# 运行测试
./ui_test
```

## 开发状态

### 已完成

- ✅ 核心UI运行时架构
- ✅ 基础UI组件（Text, Button, VStack, HStack）
- ✅ duorou_gui 后端适配
- ✅ Mock后端实现
- ✅ 布局系统
- ✅ 样式系统（颜色、字体、边距）
- ✅ CMake集成

### 开发中

- 🔄 UIIntegration（Swift语言集成）
- 🔄 更多UI组件（Image, Grid, ScrollView等）
- 🔄 动画系统
- 🔄 主题系统

### 计划中

- 📋 事件系统增强
- 📋 数据绑定
- 📋 自定义组件支持
- 📋 Web后端支持

## 测试

运行UI系统测试：

```bash
cd /path/to/miniswift
make -j4
./build/ui_test
```

测试覆盖：
- UI应用初始化
- 后端检测和选择
- UI组件创建和渲染
- 布局系统
- 样式系统
- 几何类型

## 贡献

UI系统是MiniSwift项目的重要组成部分，欢迎贡献代码和建议。在开发新功能时，请确保：

1. 遵循现有的代码风格
2. 添加适当的测试
3. 更新相关文档
4. 确保跨平台兼容性

## 许可证

本UI系统遵循MiniSwift项目的许可证条款。
