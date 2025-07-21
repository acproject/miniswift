# MiniSwift 访问控制实现计划

## 概述

本文档详细规划了为 MiniSwift 实现 Swift 访问控制特性的具体方案。访问控制是第二阶段高级 OOP 特性的重要组成部分，将为代码提供封装性和模块化支持。

## Swift 访问控制机制分析

### 访问级别层次

Swift 提供六种访问级别（从最开放到最严格）：

1. **open** - 最开放，允许跨模块继承和重写（仅适用于类和类成员）
2. **public** - 公开访问，可跨模块使用但不能继承/重写
3. **package** - 包级访问，同一包内的所有模块可访问
4. **internal** - 内部访问，同一模块内可访问（默认级别）
5. **fileprivate** - 文件私有，同一源文件内可访问
6. **private** - 私有，仅在声明的作用域内可访问

### 核心原则

- **最小权限原则**：实体不能定义为比其依赖类型更开放的访问级别
- **默认内部访问**：未明确指定时默认为 internal
- **类型成员继承**：成员的默认访问级别受类型访问级别影响

## 实现架构设计

### 1. 词法分析扩展

#### Token 类型扩展

在 `Token.h` 中添加访问控制关键字：

```cpp
enum class TokenType {
  // 现有 tokens...
  
  // Access Control Keywords
  Open,
  Public,
  Package,
  Internal,
  Fileprivate,
  Private,
  
  // 现有 tokens...
};
```

#### 词法分析器更新

在 `Lexer.cpp` 中添加关键字识别：

```cpp
// 在关键字映射中添加
{"open", TokenType::Open},
{"public", TokenType::Public},
{"package", TokenType::Package},
{"internal", TokenType::Internal},
{"fileprivate", TokenType::Fileprivate},
{"private", TokenType::Private},
```

### 2. AST 节点扩展

#### 访问级别枚举

创建新文件 `src/parser/AccessControl.h`：

```cpp
#ifndef MINISWIFT_ACCESS_CONTROL_H
#define MINISWIFT_ACCESS_CONTROL_H

namespace miniswift {

enum class AccessLevel {
  PRIVATE = 0,
  FILEPRIVATE = 1,
  INTERNAL = 2,
  PACKAGE = 3,
  PUBLIC = 4,
  OPEN = 5
};

// 访问级别比较函数
bool isMoreRestrictive(AccessLevel a, AccessLevel b);
bool isLessRestrictive(AccessLevel a, AccessLevel b);
std::string accessLevelToString(AccessLevel level);
AccessLevel tokenToAccessLevel(TokenType token);

// 访问控制上下文
struct AccessContext {
  std::string moduleName;
  std::string packageName;
  std::string fileName;
  
  AccessContext(const std::string& module, const std::string& package, const std::string& file)
    : moduleName(module), packageName(package), fileName(file) {}
};

} // namespace miniswift

#endif // MINISWIFT_ACCESS_CONTROL_H
```

#### AST 节点修改

为所有可访问控制的实体添加访问级别字段：

```cpp
// 在 StmtNodes.h 中修改现有结构

// 为 StructMember 添加访问控制
struct StructMember {
  Token name;
  Token type;
  std::unique_ptr<Expr> defaultValue;
  bool isVar;
  bool isStatic;
  bool isLazy;
  AccessLevel accessLevel = AccessLevel::INTERNAL; // 新增
  AccessLevel setterAccessLevel = AccessLevel::INTERNAL; // getter/setter 分离访问控制
  
  // 现有成员...
};

// 为 StructStmt 添加访问控制
struct StructStmt : Stmt {
  Token name;
  AccessLevel accessLevel = AccessLevel::INTERNAL; // 新增
  std::vector<StructMember> members;
  // 现有成员...
};

// 为 ClassStmt 添加访问控制
struct ClassStmt : Stmt {
  Token name;
  Token superclass;
  AccessLevel accessLevel = AccessLevel::INTERNAL; // 新增
  std::vector<StructMember> members;
  // 现有成员...
};

// 为 FunctionStmt 添加访问控制
struct FunctionStmt : Stmt {
  Token name;
  AccessLevel accessLevel = AccessLevel::INTERNAL; // 新增
  std::vector<Parameter> parameters;
  // 现有成员...
};

// 为 VarStmt 添加访问控制
struct VarStmt : Stmt {
  Token name;
  AccessLevel accessLevel = AccessLevel::INTERNAL; // 新增
  AccessLevel setterAccessLevel = AccessLevel::INTERNAL; // 新增
  std::unique_ptr<Expr> initializer;
  // 现有成员...
};
```

### 3. 解析器扩展

#### Parser.h 扩展

```cpp
class Parser {
public:
  // 现有方法...
  
private:
  // 新增访问控制解析方法
  AccessLevel parseAccessLevel();
  bool isAccessLevelToken(TokenType type);
  void parseAccessControlModifiers(AccessLevel& accessLevel, AccessLevel& setterAccessLevel);
  
  // 现有成员...
};
```

#### 解析逻辑实现

在 `Parser.cpp` 中实现访问控制解析：

```cpp
AccessLevel Parser::parseAccessLevel() {
  if (match({TokenType::Open})) return AccessLevel::OPEN;
  if (match({TokenType::Public})) return AccessLevel::PUBLIC;
  if (match({TokenType::Package})) return AccessLevel::PACKAGE;
  if (match({TokenType::Internal})) return AccessLevel::INTERNAL;
  if (match({TokenType::Fileprivate})) return AccessLevel::FILEPRIVATE;
  if (match({TokenType::Private})) return AccessLevel::PRIVATE;
  return AccessLevel::INTERNAL; // 默认级别
}

void Parser::parseAccessControlModifiers(AccessLevel& accessLevel, AccessLevel& setterAccessLevel) {
  accessLevel = parseAccessLevel();
  setterAccessLevel = accessLevel;
  
  // 处理 private(set), fileprivate(set) 等语法
  if (check(TokenType::LParen)) {
    advance(); // consume '('
    if (match({TokenType::Set})) {
      consume(TokenType::RParen, "Expected ')' after 'set'");
      setterAccessLevel = accessLevel;
      accessLevel = parseAccessLevel(); // 重新解析 getter 的访问级别
    }
  }
}
```

### 4. 语义分析和访问控制检查

#### 访问控制检查器

创建 `src/interpreter/AccessControlChecker.h`：

```cpp
#ifndef MINISWIFT_ACCESS_CONTROL_CHECKER_H
#define MINISWIFT_ACCESS_CONTROL_CHECKER_H

#include "../parser/AccessControl.h"
#include "../parser/AST.h"
#include <unordered_map>
#include <string>

namespace miniswift {

class AccessControlChecker {
public:
  AccessControlChecker(const AccessContext& context);
  
  // 检查访问权限
  bool canAccess(const std::string& entityName, AccessLevel entityLevel, 
                 const AccessContext& accessContext);
  
  // 检查类型定义的访问控制一致性
  void checkTypeDefinition(const StructStmt& stmt);
  void checkTypeDefinition(const ClassStmt& stmt);
  
  // 检查成员访问
  void checkMemberAccess(const MemberAccess& expr, const AccessContext& context);
  
  // 检查继承访问控制
  void checkInheritance(const ClassStmt& subclass, const ClassStmt& superclass);
  
  // 验证访问级别一致性
  void validateAccessLevelConsistency(AccessLevel entityLevel, AccessLevel dependencyLevel,
                                     const std::string& entityName, const std::string& dependencyName);
  
private:
  AccessContext context_;
  std::unordered_map<std::string, AccessLevel> typeAccessLevels_;
  std::unordered_map<std::string, AccessLevel> memberAccessLevels_;
};

} // namespace miniswift

#endif // MINISWIFT_ACCESS_CONTROL_CHECKER_H
```

#### 实现访问控制检查逻辑

在 `AccessControlChecker.cpp` 中实现：

```cpp
bool AccessControlChecker::canAccess(const std::string& entityName, AccessLevel entityLevel,
                                    const AccessContext& accessContext) {
  switch (entityLevel) {
    case AccessLevel::OPEN:
    case AccessLevel::PUBLIC:
      return true; // 总是可访问
      
    case AccessLevel::PACKAGE:
      return context_.packageName == accessContext.packageName;
      
    case AccessLevel::INTERNAL:
      return context_.moduleName == accessContext.moduleName;
      
    case AccessLevel::FILEPRIVATE:
      return context_.fileName == accessContext.fileName;
      
    case AccessLevel::PRIVATE:
      // 需要更复杂的作用域检查
      return checkPrivateScope(entityName, accessContext);
  }
  return false;
}

void AccessControlChecker::validateAccessLevelConsistency(AccessLevel entityLevel, 
                                                         AccessLevel dependencyLevel,
                                                         const std::string& entityName, 
                                                         const std::string& dependencyName) {
  if (isMoreRestrictive(dependencyLevel, entityLevel)) {
    throw std::runtime_error("Access control error: " + entityName + 
                           " cannot be more accessible than " + dependencyName);
  }
}
```

### 5. 解释器集成

#### Environment 扩展

在 `Environment.h` 中添加访问控制支持：

```cpp
class Environment {
public:
  // 现有方法...
  
  // 新增访问控制方法
  void defineWithAccessLevel(const std::string& name, const Value& value, AccessLevel level);
  bool canAccess(const std::string& name, const AccessContext& context);
  AccessLevel getAccessLevel(const std::string& name);
  
private:
  std::unordered_map<std::string, AccessLevel> accessLevels_;
  // 现有成员...
};
```

#### Interpreter 扩展

在 `Interpreter.h` 中集成访问控制检查：

```cpp
class Interpreter : public ExprVisitor, public StmtVisitor {
public:
  // 现有方法...
  
  void setAccessContext(const AccessContext& context);
  
private:
  AccessControlChecker accessChecker_;
  AccessContext currentContext_;
  
  // 在访问成员时进行检查
  void visit(const MemberAccess& expr) override;
  void visit(const StructStmt& stmt) override;
  void visit(const ClassStmt& stmt) override;
  // 现有成员...
};
```

## 实现阶段规划

### 阶段 1：基础设施（1-2 天）

1. **词法分析扩展**
   - 添加访问控制关键字到 Token.h
   - 更新 Lexer.cpp 识别新关键字
   - 编写基础测试用例

2. **AST 节点扩展**
   - 创建 AccessControl.h 定义访问级别
   - 为现有 AST 节点添加访问级别字段
   - 实现访问级别比较函数

### 阶段 2：解析器扩展（2-3 天）

1. **解析逻辑实现**
   - 实现 parseAccessLevel() 方法
   - 支持 private(set) 等复合语法
   - 更新所有声明解析方法

2. **语法验证**
   - 验证访问控制修饰符的正确位置
   - 检查语法一致性
   - 错误处理和报告

### 阶段 3：语义分析（3-4 天）

1. **访问控制检查器**
   - 实现 AccessControlChecker 类
   - 基本访问权限检查
   - 类型定义一致性检查

2. **高级检查**
   - 继承访问控制检查
   - 协议遵循访问控制
   - 扩展访问控制

### 阶段 4：解释器集成（2-3 天）

1. **运行时检查**
   - Environment 访问控制支持
   - 成员访问检查
   - 方法调用检查

2. **错误处理**
   - 访问控制违规错误报告
   - 友好的错误消息
   - 调试信息支持

### 阶段 5：测试和优化（2-3 天）

1. **全面测试**
   - 单元测试覆盖所有访问级别
   - 集成测试验证复杂场景
   - 边界情况测试

2. **性能优化**
   - 访问检查缓存
   - 编译时优化
   - 内存使用优化

## 测试策略

### 测试用例设计

创建 `tests/oop/phase2/access_control_test.swift`：

```swift
// 基础访问控制测试
public class PublicClass {
    public var publicProperty = "public"
    internal var internalProperty = "internal"
    fileprivate var fileprivateProperty = "fileprivate"
    private var privateProperty = "private"
    
    public func publicMethod() {
        print("public method")
    }
    
    private func privateMethod() {
        print("private method")
    }
}

// Getter/Setter 访问控制测试
struct TrackedString {
    private(set) var numberOfEdits = 0
    var value: String = "" {
        didSet {
            numberOfEdits += 1
        }
    }
}

// 继承访问控制测试
open class OpenBaseClass {
    open func openMethod() {}
    public func publicMethod() {}
}

public class DerivedClass: OpenBaseClass {
    override public func openMethod() {} // 可以重写
    // override public func publicMethod() {} // 错误：不能重写 public 方法
}

// 访问控制错误测试
// private class PrivateClass {}
// public var invalidProperty: PrivateClass // 错误：public 变量不能使用 private 类型
```

### 测试覆盖范围

1. **基础访问级别**
   - 所有六种访问级别的基本功能
   - 默认访问级别行为
   - 访问级别继承规则

2. **复合语法**
   - private(set), fileprivate(set) 等
   - 访问级别组合
   - 语法错误处理

3. **语义检查**
   - 访问级别一致性
   - 继承访问控制
   - 跨模块访问

4. **运行时行为**
   - 成员访问检查
   - 方法调用权限
   - 错误报告质量

## 错误处理策略

### 编译时错误

1. **语法错误**
   - 无效的访问控制修饰符位置
   - 不支持的访问级别组合
   - 语法不完整

2. **语义错误**
   - 访问级别不一致
   - 违反访问控制原则
   - 继承访问控制冲突

### 运行时错误

1. **访问违规**
   - 尝试访问私有成员
   - 跨模块访问内部成员
   - 文件外访问文件私有成员

2. **错误消息设计**
   - 清晰的错误描述
   - 建议的修复方案
   - 相关代码位置信息

## 性能考虑

### 编译时优化

1. **访问检查缓存**
   - 缓存访问控制决策
   - 避免重复计算
   - 智能缓存失效

2. **静态分析**
   - 编译时确定访问权限
   - 消除运行时检查
   - 死代码消除

### 运行时优化

1. **快速路径**
   - public/open 成员快速访问
   - 同模块访问优化
   - 内联访问检查

2. **内存效率**
   - 紧凑的访问级别存储
   - 共享访问控制信息
   - 延迟访问检查

## 未来扩展

### 模块系统集成

1. **模块边界**
   - 明确的模块定义
   - 跨模块导入控制
   - 模块访问策略

2. **包管理**
   - 包级访问控制
   - 依赖访问管理
   - 版本兼容性

### 高级特性

1. **条件访问控制**
   - 基于编译条件的访问控制
   - 平台特定访问级别
   - 调试模式访问控制

2. **访问控制分析工具**
   - 访问控制可视化
   - 访问模式分析
   - 重构建议

## 总结

访问控制的实现将显著提升 MiniSwift 的封装性和模块化能力。通过分阶段的实现策略，我们可以确保：

1. **功能完整性**：支持 Swift 的所有访问控制特性
2. **性能效率**：最小化运行时开销
3. **易用性**：提供清晰的错误消息和调试支持
4. **可扩展性**：为未来的模块系统和高级特性做好准备

预计总实现时间：**10-15 天**，这将为 MiniSwift 的面向对象特性奠定坚实的基础。

---

*本文档将随着实现进度持续更新和完善*