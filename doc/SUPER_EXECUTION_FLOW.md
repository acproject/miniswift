# test_super.swift 执行流程分析

## 概述

本文档详细分析了 `test_super.swift` 文件的执行流程，包括涉及的源代码模块、执行路径和关键组件。

## 测试文件内容

```swift
// 测试super关键字功能

class Animal {
    var name: String = "Unknown"
    
    func speak() {
        print("Animal makes a sound")
    }
    
    func getName() -> String {
        return name
    }
}

class Dog : Animal {
    override func speak() {
        super.speak()  // 调用父类方法
        print("Dog barks")
    }
    
    override func getName() -> String {
        let parentName = super.getName()  // 调用父类方法获取名称
        return "Dog named " + parentName
    }
}

let dog = Dog()
dog.name = "Buddy"
print("Name: " + dog.getName())
dog.speak()
```

## 执行流程概览

### 1. 程序入口 (main.cpp)

**文件路径**: `src/main.cpp`

执行流程从 `main()` 函数开始：

1. **命令行参数解析**: 解析编译选项（语义分析、LLVM代码生成等）
2. **文件读取**: 通过 `runFile()` 函数读取 `test_super.swift` 文件内容
3. **调用核心处理函数**: 调用 `run()` 函数处理源代码

### 2. 词法分析阶段 (Lexer)

**涉及文件**:
- `src/lexer/Lexer.h`
- `src/lexer/Lexer.cpp`
- `src/lexer/Token.h`

**处理过程**:
1. **创建词法分析器**: `miniswift::Lexer lexer(source)`
2. **扫描tokens**: `lexer.scanTokens()`
3. **关键字识别**: 识别 `class`, `override`, `super`, `func`, `var` 等关键字
4. **super关键字**: 在 `Token.h` 中定义为 `TokenType::Super`

**关键代码路径**:
```cpp
// main.cpp 中的调用
miniswift::Lexer lexer(source);
std::vector<miniswift::Token> tokens = lexer.scanTokens();
```

### 3. 语法分析阶段 (Parser)

**涉及文件**:
- `src/parser/Parser.h`
- `src/parser/Parser.cpp`
- `src/parser/AST.h`
- `src/parser/StmtNodes.h`
- `src/parser/ExprNodes.h`

**处理过程**:
1. **创建语法分析器**: `miniswift::Parser parser(tokens)`
2. **解析语句**: `parser.parse()` 返回AST节点列表
3. **类声明解析**: 解析 `Animal` 和 `Dog` 类声明
4. **继承关系解析**: 解析 `Dog : Animal` 继承关系
5. **super表达式解析**: 解析 `super.speak()` 和 `super.getName()` 调用

**关键AST节点**:
- `ClassStmt`: 表示类声明（Animal, Dog）
- `Super`: 表示super表达式（定义在 `ExprNodes.h` 第372行）
- `MemberAccess`: 表示成员访问
- `Call`: 表示方法调用

**Super表达式定义**:
```cpp
// ExprNodes.h
struct Super : Expr {
  Token keyword; // The 'super' token
  Token method;  // The method or property name

  Super(Token keyword, Token method) : keyword(keyword), method(method) {}
  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }
};
```

### 4. 语义分析阶段 (可选)

**涉及文件**:
- `src/semantic/SemanticAnalyzer.h`
- `src/semantic/SemanticAnalyzer.cpp`
- `src/semantic/TypeSystem.h`
- `src/semantic/SymbolTable.h`

**处理过程**:
1. **类型检查**: 验证类的继承关系
2. **方法重写验证**: 检查 `override` 关键字的正确使用
3. **super调用验证**: 验证super调用的合法性
4. **符号表管理**: 管理类、方法、属性的符号信息

### 5. 解释执行阶段 (Interpreter)

**涉及文件**:
- `src/interpreter/Interpreter.h`
- `src/interpreter/Interpreter.cpp`
- `src/interpreter/OOP/Inheritance.h`
- `src/interpreter/OOP/Inheritance.cpp`
- `src/interpreter/Environment.h`
- `src/interpreter/Value.h`

**核心组件**:

#### 5.1 继承管理器 (InheritanceManager)

**功能**:
- 注册类的继承关系
- 管理方法的重写
- 支持方法的继承查找

**关键方法**:
```cpp
void registerClass(const std::string &className, const std::string &superClassName);
std::shared_ptr<FunctionStmt> findMethod(const std::string &className, const std::string &methodName);
bool isMethodOverridden(const std::string &className, const std::string &methodName);
```

#### 5.2 Super处理器 (SuperHandler)

**功能**:
- 处理super方法调用
- 处理super属性访问
- 管理super调用的上下文

**关键方法**:
```cpp
Value callSuperMethod(const std::string &currentClass, const std::string &methodName, 
                     const std::vector<Value> &arguments, std::shared_ptr<Environment> environment);
Value getSuperProperty(const std::string &currentClass, const std::string &propertyName, 
                      std::shared_ptr<Environment> environment);
```

#### 5.3 解释器访问者模式

**Super表达式处理**:
```cpp
// Interpreter.h 中的声明
void visit(const Super& expr) override;

// 实际处理逻辑在 Interpreter.cpp 中实现
```

### 6. 执行步骤详解

#### 6.1 类定义阶段

1. **Animal类注册**:
   - 注册类名 "Animal"
   - 注册方法 "speak", "getName"
   - 注册属性 "name"

2. **Dog类注册**:
   - 注册类名 "Dog"
   - 注册继承关系 "Dog" -> "Animal"
   - 注册重写方法 "speak", "getName"

#### 6.2 实例创建阶段

1. **创建Dog实例**: `let dog = Dog()`
   - 调用Dog的初始化器
   - 继承Animal的属性和方法
   - 建立实例的类型信息

2. **属性赋值**: `dog.name = "Buddy"`
   - 访问继承的name属性
   - 设置属性值

#### 6.3 方法调用阶段

1. **getName()调用**: `dog.getName()`
   - 查找Dog类的getName方法
   - 执行重写的getName方法
   - 在方法内部调用 `super.getName()`
   - SuperHandler处理super调用，找到Animal类的getName方法
   - 返回组合后的字符串

2. **speak()调用**: `dog.speak()`
   - 查找Dog类的speak方法
   - 执行重写的speak方法
   - 在方法内部调用 `super.speak()`
   - SuperHandler处理super调用，执行Animal类的speak方法
   - 继续执行Dog类的剩余代码

### 7. 关键数据结构

#### 7.1 环境 (Environment)

**文件**: `src/interpreter/Environment.h`

**功能**:
- 管理变量和方法的作用域
- 支持嵌套作用域
- 处理this/self引用

#### 7.2 值系统 (Value)

**文件**: `src/interpreter/Value.h`

**功能**:
- 表示运行时值
- 支持类实例、基本类型等
- 管理对象的生命周期

#### 7.3 类实例 (ClassInstance)

**功能**:
- 表示类的实例
- 存储实例属性
- 维护类型信息和继承关系

### 8. 错误处理

**涉及文件**:
- `src/interpreter/ErrorHandling.h`
- `src/interpreter/ErrorHandling.cpp`

**处理场景**:
- super调用在非子类中使用
- 调用不存在的父类方法
- 类型不匹配错误

### 9. 内存管理

**涉及文件**:
- `src/interpreter/Memory/` (如果存在)

**管理内容**:
- 类实例的内存分配
- 继承关系的内存布局
- 垃圾回收（如果实现）

## 总结

`test_super.swift` 的执行涉及了MiniSwift编译器的所有主要组件：

1. **词法分析**: 识别关键字和操作符
2. **语法分析**: 构建AST，解析类继承和super表达式
3. **语义分析**: 验证继承关系和方法重写
4. **解释执行**: 通过继承管理器和super处理器实现运行时行为

整个流程展示了面向对象编程中继承和方法重写的完整实现，特别是super关键字的处理机制。这个测试用例很好地验证了MiniSwift对Swift语言OOP特性的支持程度。

## 相关文档

- [OOP增强计划](OOP_ENHANCEMENT_PLAN.md)
- [语法指南](SYNTAX_GUIDE.md)
- [编译流程分析](COMPILATION_FLOW_ANALYSIS.md)