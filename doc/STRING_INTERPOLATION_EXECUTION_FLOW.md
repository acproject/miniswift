# MiniSwift 字符串插值执行流程文档

本文档详细说明了 MiniSwift 项目如何执行以下两个语句：

```swift
let name = "zhangli"
print("Hello \(name) !")
```

## 1. 整体架构概述

MiniSwift 项目采用经典的解释器架构，包含以下主要组件：

- **词法分析器 (Lexer)**: 将源代码转换为 Token 流
- **语法分析器 (Parser)**: 将 Token 流转换为抽象语法树 (AST)
- **解释器 (Interpreter)**: 遍历 AST 并执行代码
- **环境管理 (Environment)**: 管理变量作用域和存储
- **值系统 (Value)**: 表示运行时的各种数据类型

## 2. 执行流程详解

### 2.1 第一个语句：`let name = "zhangli"`

#### 2.1.1 词法分析阶段

**文件**: `src/lexer/Lexer.cpp`

词法分析器将源代码分解为以下 Token 序列：

```
TokenType::Let      "let"
TokenType::Identifier "name"
TokenType::Equal    "="
TokenType::StringLiteral "zhangli"
```

关键实现：
- `Lexer::scanTokens()` 方法扫描整个源代码
- `Lexer::scanToken()` 方法识别单个 Token
- `Lexer::stringLiteral()` 方法处理字符串字面量
- `Lexer::identifier()` 方法识别标识符和关键字

#### 2.1.2 语法分析阶段

**文件**: `src/parser/Parser.cpp`

解析器将 Token 序列转换为 AST 节点：

```cpp
// 在 Parser::declaration() 中识别 let 关键字
if (match({TokenType::Var, TokenType::Let})) {
    bool isConst = previous().type == TokenType::Let;  // true
    // 解析变量名
    consume(TokenType::Identifier, "Expect variable name.");
    Token name = previous();  // "name"
    // 解析初始化表达式
    if (match({TokenType::Equal})) {
        initializer = expression();  // 解析 "zhangli"
    }
    // 创建 VarStmt AST 节点
    return std::make_unique<VarStmt>(name, std::move(initializer), isConst, type);
}
```

生成的 AST 节点类型：`VarStmt`

**文件**: `src/parser/StmtNodes.h`

```cpp
struct VarStmt : Stmt {
    const Token name;                    // "name"
    const std::unique_ptr<Expr> initializer;  // Literal("zhangli")
    const bool isConst;                  // true
    const Token type;                    // 空（类型推断）
};
```

#### 2.1.3 解释执行阶段

**文件**: `src/interpreter/Interpreter.cpp`

```cpp
void Interpreter::visit(const VarStmt& stmt) {
    Value value; // 默认初始化为 nil
    if (stmt.initializer) {
        value = evaluate(*stmt.initializer);  // 计算 "zhangli"
    }
    // 在环境中定义变量
    environment->define(stmt.name.lexeme, value, stmt.isConst, stmt.type.lexeme);
}
```

执行步骤：
1. 调用 `evaluate()` 计算初始化表达式
2. 对于字符串字面量，在 `visit(const Literal& expr)` 中处理
3. 创建 `Value("zhangli")` 对象
4. 调用 `Environment::define()` 将变量存储到环境中

**文件**: `src/interpreter/Environment.cpp`

```cpp
void Environment::define(const std::string& name, const Value& value, bool isConst, const std::string& typeName) {
    values[name] = {value, isConst, typeName};
}
```

### 2.2 第二个语句：`print("Hello \(name) !")`

#### 2.2.1 词法分析阶段

**当前实现的限制**：字符串插值的词法分析存在简化实现。

理想的 Token 序列应该是：
```
TokenType::Print                "print"
TokenType::LParen              "("
TokenType::StringLiteral       "Hello "
TokenType::InterpolationStart  "\("
TokenType::Identifier          "name"
TokenType::RParen              ")"
TokenType::StringLiteral       " !"
TokenType::RParen              ")"
```

**实际实现**：在 `Lexer.cpp` 中，字符串插值的处理逻辑：

```cpp
Token Lexer::stringLiteral() {
    std::string value;
    while (peek() != '"' && current < source.length()) {
        if (peek() == '\\' && peekNext() == '(') {
            // 检测到插值开始
            if (!value.empty()) {
                return {TokenType::InterpolatedStringLiteral, value, line};
            }
            advance(); // 消费 \\
            advance(); // 消费 (
            inInterpolation = true;
            interpolationDepth = 1;
            inStringLiteral = false;
            return {TokenType::InterpolationStart, "\\(", line};
        }
        // 处理其他字符...
    }
}
```

#### 2.2.2 语法分析阶段

在 `Parser.cpp` 中，print 语句的解析：

```cpp
std::unique_ptr<Stmt> Parser::printStatement() {
    consume(TokenType::LParen, "Expect '(' after 'print'.");
    auto value = expression();  // 解析字符串插值表达式
    consume(TokenType::RParen, "Expect ')' after expression.");
    return std::make_unique<PrintStmt>(std::move(value));
}
```

**当前限制**：字符串插值的解析存在简化实现：

```cpp
// 在 Parser::primary() 中
if (match({TokenType::InterpolatedStringLiteral})) {
    // 目前将插值字符串当作普通字符串处理
    // TODO: 实现完整的字符串插值支持
    return std::make_unique<Literal>(previous());
}
```

#### 2.2.3 解释执行阶段

**文件**: `src/interpreter/Interpreter.cpp`

Print 语句的执行：

```cpp
void Interpreter::visit(const PrintStmt& stmt) {
    Value val = evaluate(*stmt.expression);  // 计算表达式值
    switch (val.type) {
        case ValueType::String:
            std::cout << std::get<std::string>(val.value) << std::endl;
            break;
        // 处理其他类型...
    }
}
```

## 3. 字符串插值的完整实现方案

### 3.1 当前实现的限制

1. **词法分析限制**：字符串插值的 Token 化不完整
2. **语法分析限制**：插值表达式没有被正确解析为复合表达式
3. **解释执行限制**：插值表达式没有被动态计算

### 3.2 理想的实现流程

#### 3.2.1 词法分析改进

需要正确处理字符串插值的状态机：

```cpp
// 伪代码
Token Lexer::handleStringInterpolation() {
    if (inStringLiteral && peek() == '\\' && peekNext() == '(') {
        // 切换到插值模式
        inInterpolation = true;
        return {TokenType::InterpolationStart, "\\(", line};
    }
    if (inInterpolation && peek() == ')') {
        // 结束插值，回到字符串模式
        inInterpolation = false;
        inStringLiteral = true;
        return {TokenType::InterpolationEnd, ")", line};
    }
}
```

#### 3.2.2 语法分析改进

需要创建新的 AST 节点类型来表示字符串插值：

```cpp
// 新的 AST 节点
struct StringInterpolation : Expr {
    std::vector<std::unique_ptr<Expr>> parts;  // 字符串片段和表达式
    std::vector<bool> isExpression;            // 标记哪些是表达式
};
```

#### 3.2.3 解释执行改进

```cpp
void Interpreter::visit(const StringInterpolation& expr) {
    std::string result;
    for (size_t i = 0; i < expr.parts.size(); ++i) {
        Value partValue = evaluate(*expr.parts[i]);
        if (expr.isExpression[i]) {
            // 将表达式结果转换为字符串
            result += valueToString(partValue);
        } else {
            // 直接添加字符串字面量
            result += std::get<std::string>(partValue.value);
        }
    }
    this->result = Value(result);
}
```

## 4. 数据流图

```
源代码
  ↓
词法分析器 (Lexer)
  ↓
Token 流
  ↓
语法分析器 (Parser)
  ↓
AST (抽象语法树)
  ↓
解释器 (Interpreter)
  ↓
环境 (Environment) ← → 值系统 (Value)
  ↓
输出结果
```

## 5. 关键文件和类的作用

| 文件路径 | 主要类/函数 | 作用 |
|---------|------------|------|
| `src/lexer/Lexer.h/.cpp` | `Lexer` | 词法分析，Token 生成 |
| `src/lexer/Token.h` | `Token`, `TokenType` | Token 定义 |
| `src/parser/Parser.h/.cpp` | `Parser` | 语法分析，AST 构建 |
| `src/parser/StmtNodes.h` | `VarStmt`, `PrintStmt` | 语句 AST 节点 |
| `src/parser/ExprNodes.h` | `Literal`, `VarExpr` | 表达式 AST 节点 |
| `src/interpreter/Interpreter.h/.cpp` | `Interpreter` | AST 遍历和执行 |
| `src/interpreter/Environment.h/.cpp` | `Environment` | 变量作用域管理 |
| `src/interpreter/Value.h/.cpp` | `Value` | 运行时值表示 |

## 6. 总结

MiniSwift 项目通过经典的三阶段编译器架构实现了基本的 Swift 语法支持。对于给定的两个语句：

1. **变量声明** (`let name = "zhangli"`) 通过完整的词法分析 → 语法分析 → 解释执行流程正确处理
2. **字符串插值** (`print("Hello \(name) !")`) 目前存在简化实现，需要进一步完善

项目的模块化设计使得各个组件职责清晰，便于扩展和维护。字符串插值功能的完整实现需要在词法分析、语法分析和解释执行三个阶段都进行相应的改进。