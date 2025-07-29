# MiniSwift

一个用 C++ 实现的简化版 Swift 解释器，支持基本的 Swift 语法功能。

## 功能特性

✅ **已实现功能**
- 变量声明（`let` 和 `var`）
- 基本数据类型（字符串、整数）
- 字符串插值语法（`"Hello \(name)"`）
- 打印语句（`print()`）
- 基本表达式求值
- 词法分析和语法解析
- **UI库框架** - 基于GTK4的SwiftUI风格UI库
  - 基础组件（Text, Button, Image, Rectangle等）
  - 布局容器（VStack, HStack, ZStack, Grid）
  - 视图修饰符（padding, background, foregroundColor等）
  - 样式系统（Color, Font, Alignment等）

🚧 **部分实现**
- 字符串插值（解析完成，求值待完善）
- 数学运算

📋 **计划功能**
- 控制流语句（if/else, for, while）
- 函数定义和调用
- 类和结构体
- 数组和字典
- **UI库增强**
  - GTK4后端完整实现
  - 动画和过渡效果
  - 事件处理系统
  - 更多UI组件（List, NavigationView, TabView等）

## 快速开始

### 编译项目

#### Unix/Linux/macOS

```bash
# 创建构建目录
mkdir -p build
cd build

# 使用 CMake 配置和编译
cmake ..
make
```

#### Windows

**方法 1: 使用构建脚本 (推荐)**
```cmd
# 运行 Windows 构建脚本
scripts\build_windows.bat

# 或构建 Debug 版本
scripts\build_windows.bat debug
```

**方法 2: 手动构建**
```cmd
# 创建构建目录
mkdir build_windows
cd build_windows

# 配置项目 (需要预先安装 vcpkg 和 LLVM)
cmake .. -DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake # linux 可以增加 -DVCPKG_TARGET_TRIPLET=x64-windows-static

# 构建项目
cmake --build . --config Release --parallel 
```

**详细的 Windows 构建指南请参考**: [WINDOWS_BUILD_GUIDE.md](doc/WINDOWS_BUILD_GUIDE.md)

### 运行示例

#### 交互式使用

**Unix/Linux/macOS:**
```bash
# 直接运行解释器
./build/miniswift

# 输入 Swift 代码
let name = "World"
print("Hello \(name)!")
```

**Windows:**
```cmd
# 直接运行解释器
build_windows\Release\miniswift.exe

# 输入 Swift 代码
let name = "World"
print("Hello \(name)!")
```

#### 运行文件

**Unix/Linux/macOS:**
```bash
# 从文件读取代码
cat your_program.swift | ./build/miniswift
```

**Windows:**
```cmd
# 从文件读取代码
type your_program.swift | build_windows\Release\miniswift.exe
```

#### 使用 Playground
```bash
# 运行预设的测试用例
./test_playground.sh

# 或运行简化版测试
cat simple_playground.swift | ./build/miniswift

# 交互式语法演示
./interactive_demo.sh
```

#### 退出交互模式
```bash
# 在交互模式中输入以下任一命令退出：
exit
quit
q
```

## 语法示例

### 变量声明
```swift
let name = "Swift"        // 常量
var age = 25             // 变量
```

### 字符串插值
```swift
let greeting = "Hello \(name)!"
let info = "I am \(age) years old"
```

### 打印输出
```swift
print("Hello, World!")
print(greeting)
print(age)
```

### 数学运算
```swift
let a = 10
let b = 5
let sum = a + b
let product = a * b
```

### UI开发
```swift
// 创建简单的UI界面
struct ContentView: UIView {
    var body: some UIView {
        VStack(spacing: 20) {
            Text("Hello, MiniSwift UI!")
                .font(.title)
                .foregroundColor(.blue)
            
            Button("Click Me") {
                print("Button tapped!")
            }
            .background(.blue)
            .foregroundColor(.white)
            .cornerRadius(8)
            .padding()
        }
    }
}

// 网格布局示例
struct GridExample: UIView {
    var body: some UIView {
        Grid(horizontalSpacing: 10, verticalSpacing: 10) {
            GridRow {
                Rectangle().fill(.red).frame(width: 50, height: 50)
                Rectangle().fill(.green).frame(width: 50, height: 50)
                Rectangle().fill(.blue).frame(width: 50, height: 50)
            }
        }
        .padding()
    }
}
```

## 项目结构

```
MiniSwift/
├── src/
│   ├── lexer/           # 词法分析器
│   │   ├── Lexer.cpp
│   │   ├── Lexer.h
│   │   └── Token.h
│   ├── parser/          # 语法分析器
│   │   ├── Parser.cpp
│   │   ├── Parser.h
│   │   ├── AST.h
│   │   └── AstPrinter.cpp
│   ├── interpreter/     # 解释器
│   │   ├── Interpreter.cpp
│   │   ├── Interpreter.h
│   │   ├── Environment.cpp
│   │   ├── Environment.h
│   │   └── Value.h
│   └── main.cpp         # 主程序入口
├── miniswift/           # Swift包管理器项目
│   ├── Sources/MiniSwift/
│   │   ├── MiniSwift.swift    # 主模块文件
│   │   └── UI/               # UI库模块
│   │       ├── UIView.swift      # 核心UI协议
│   │       ├── Layout.swift      # 布局容器
│   │       ├── Components.swift  # 基础组件
│   │       ├── Geometry.swift    # 几何和对齐
│   │       ├── Style.swift       # 样式系统
│   │       ├── Modifiers.swift   # 视图修饰符
│   │       └── GTK4Bridge.swift  # GTK4集成
│   └── Package.swift     # 包配置文件
├── examples/            # 示例代码
│   └── UIExample.swift  # UI库使用示例
├── build/               # 构建输出目录
├── doc/                # 文档
├── UI_README.md        # UI库详细文档
├── test_ui_basic.swift # UI库基础测试
├── playground.swift     # 完整测试用例
├── simple_playground.swift  # 简化测试用例
```

## 开发和调试

### 调试工具
项目包含多个调试程序：

```bash
# 词法分析调试
./debug_detailed

# 解析过程调试
./debug_parsing

# 字符串插值调试
./debug_interpolation
```

### 测试用例
```bash
# 运行各种测试
./test_lexer              # 词法分析测试
./test_string_edge_cases  # 字符串边界测试
./test_simple_interpolation  # 插值测试
```

## 技术架构

### 编译流程
1. **词法分析** (`Lexer`) - 将源代码转换为 Token 流
2. **语法分析** (`Parser`) - 将 Token 流转换为抽象语法树 (AST)
3. **解释执行** (`Interpreter`) - 遍历 AST 并执行代码

### 核心组件
- **Lexer**: 负责词法分析，识别关键字、标识符、字符串等
- **Parser**: 负责语法分析，构建 AST
- **Interpreter**: 负责解释执行，管理变量环境
- **Environment**: 变量作用域管理
- **Value**: 运行时值的表示

## 贡献指南

### 代码风格
- 使用 4 空格缩进
- 类名使用 PascalCase
- 函数和变量名使用 camelCase
- 添加适当的注释

### 提交代码
1. Fork 项目
2. 创建功能分支
3. 编写测试用例
4. 确保所有测试通过
5. 提交 Pull Request

## 已知问题

1. **字符串插值求值**: 目前只能解析插值语法，但不能正确求值插值表达式
   - 示例：`"Hello \(name)"` 只会输出 `"Hello "` 
   - 插值表达式 `\(name)` 被解析但不被求值

2. **词法分析问题**: 某些字符串内容被错误识别为变量
   - 示例：`"I am \(age) years old"` 中的 `years` 被识别为未定义变量
   - 中文标点符号可能导致解析错误

3. **错误处理**: 错误消息不够友好，缺少行号和列号信息

4. **表达式求值**: 复杂表达式的求值还不完整

### 语法注意事项
- ✅ 正确：`"Hello \(name)"` （在字符串字面量内使用插值）
- ❌ 错误：`print(s\(n))` （在函数调用中直接使用插值语法）
- 详细语法请参考 [SYNTAX_GUIDE.md](doc/SYNTAX_GUIDE.md)

详细的改进建议请参考 [CODE_QUALITY_SUGGESTIONS.md](doc/CODE_QUALITY_SUGGESTIONS.md)

## 许可证

MIT License

## 联系方式
acproject@qq.com
如有问题或建议，请创建 Issue 或 Pull Request。