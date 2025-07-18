# MiniSwift Control Flow Guide

本文档描述了 MiniSwift 中实现的控制流功能。

## 支持的控制流语句

### 1. 条件语句 (if-else)

#### 基本 if 语句
```swift
if condition {
    // 执行代码
}
```

#### if-else 语句
```swift
if condition {
    // 条件为真时执行
} else {
    // 条件为假时执行
}
```

#### else if 链
```swift
if condition1 {
    // 条件1为真时执行
} else if condition2 {
    // 条件2为真时执行
} else {
    // 所有条件都为假时执行
}
```

### 2. 循环语句

#### while 循环
```swift
while condition {
    // 重复执行的代码
}
```

#### for 循环
```swift
for (initializer; condition; increment) {
    // 循环体
}
```

示例：
```swift
for (var i = 0; i < 5; i = i + 1) {
    print(i)
}
```

### 3. 代码块 (Block Statements)

```swift
{
    // 代码块创建新的作用域
    var localVar = "local"
    print(localVar)
}
// localVar 在此处不可访问
```

## 作用域管理

### 块级作用域
- 每个代码块 `{}` 创建新的作用域
- 内部作用域可以访问外部变量
- 外部作用域无法访问内部变量
- 支持变量遮蔽 (Variable Shadowing)

### 循环作用域
- for 循环的初始化变量在循环结束后不可访问
- 循环体内定义的变量仅在循环体内有效

## 条件表达式

### 支持的比较运算符
- `>` 大于
- `>=` 大于等于
- `<` 小于
- `<=` 小于等于
- `==` 等于
- `!=` 不等于

### 支持的逻辑运算符
- `!` 逻辑非

### 真值判断
- `true` 为真
- `false` 为假
- `nil` 为假
- 其他所有值为真

## 与其他功能的集成

### 数组处理
```swift
var arr = [1, 2, 3, 4, 5]
for (var i = 0; i < 5; i = i + 1) {
    var element = arr[i]
    if element > 3 {
        print("Large: ")
        print(element)
    }
}
```

### 字典处理
```swift
var dict = ["key1": 10, "key2": 20]
var value = dict["key1"]
if value > 15 {
    print("High value")
} else {
    print("Low value")
}
```

### 嵌套结构
```swift
var matrix = [[1, 2], [3, 4]]
for (var row = 0; row < 2; row = row + 1) {
    var currentRow = matrix[row]
    for (var col = 0; col < 2; col = col + 1) {
        var element = currentRow[col]
        print(element)
    }
}
```

## 测试文件

以下测试文件验证了控制流功能：

1. **control_flow_test.swift** - 基本控制流测试
   - if-else 语句
   - while 循环
   - for 循环
   - 块级作用域

2. **advanced_control_flow_test.swift** - 高级控制流测试
   - else if 链
   - 嵌套循环
   - 复杂条件
   - 变量遮蔽

3. **comprehensive_control_flow_test.swift** - 综合功能测试
   - 控制流与数组的结合
   - 控制流与字典的结合
   - 嵌套数据结构处理
   - 字符串处理

## 技术实现

### AST 节点
- `BlockStmt` - 代码块语句
- `IfStmt` - if 语句
- `WhileStmt` - while 循环
- `ForStmt` - for 循环

### 解析器扩展
- 添加了控制流语句的解析方法
- 支持嵌套结构的解析
- 正确处理作用域边界

### 解释器扩展
- 实现了控制流语句的执行逻辑
- 添加了环境管理用于作用域控制
- 支持条件求值和循环控制

## 注意事项

1. **逻辑运算符限制**：目前不支持 `&&` 和 `||` 运算符
2. **break/continue**：暂不支持 break 和 continue 语句
3. **switch 语句**：暂不支持 switch-case 语句
4. **guard 语句**：暂不支持 guard 语句

## 使用示例

```swift
// 综合示例：查找数组中的最大值
var numbers = [3, 7, 2, 9, 1]
var max = numbers[0]
var maxIndex = 0

for (var i = 1; i < 5; i = i + 1) {
    var current = numbers[i]
    if current > max {
        max = current
        maxIndex = i
    }
}

print("Maximum value: ")
print(max)
print(" at index: ")
print(maxIndex)
```

这个实现为 MiniSwift 提供了完整的基础控制流功能，支持条件判断、循环控制和作用域管理。