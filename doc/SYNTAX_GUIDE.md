# MiniSwift 语法指南

## 基本语法

### 变量声明
```swift
// 常量声明
let name = "Swift"
let age = 25
let pi = 3.14159

// 变量声明
var count = 0
var message = "Hello"
```

### 字符串和字符串插值

#### 基本字符串
```swift
let greeting = "Hello, World!"
let empty = ""
```

#### 字符串插值（正确语法）
```swift
let name = "Swift"
let greeting = "Hello \(name)!"  // ✅ 正确：在字符串内部使用 \(变量名)
print(greeting)  // 输出: Hello Swift!

let age = 25
let info = "I am \(age) years old"  // ✅ 正确
print(info)  // 输出: I am 25 years old
```

#### 常见错误
```swift
let s = "hello"
let n = "world"

// ❌ 错误语法
print(s\(n))  // 这是错误的！

// ✅ 正确语法
let combined = "\(s) \(n)"  // 在字符串字面量内使用插值
print(combined)  // 输出: hello world

// 或者使用字符串连接
print(s + " " + n)  // 输出: hello world
```

### 打印语句
```swift
print("Hello, World!")  // 打印字符串字面量
print(variableName)     // 打印变量值
print("Value: \(variable)")  // 打印包含插值的字符串
```

### 数学运算
```swift
let a = 10
let b = 5
let sum = a + b      // 加法
let diff = a - b     // 减法
let product = a * b  // 乘法
let quotient = a / b // 除法
```

### 交互式命令
```swift
// 在交互式模式中可以使用的命令
exit    // 退出程序
quit    // 退出程序
q       // 退出程序
```

## 完整示例

```swift
// 声明变量
let firstName = "张"
let lastName = "三"
var age = 25

// 使用字符串插值
let fullName = "\(firstName)\(lastName)"
let introduction = "我叫\(fullName)，今年\(age)岁"

// 打印结果
print(fullName)      // 输出: 张三
print(introduction)  // 输出: 我叫张三，今年25岁

// 数学运算
let birthYear = 2024 - age
let info = "我出生于\(birthYear)年"
print(info)  // 输出: 我出生于1999年

// 退出程序
exit
```

## 注意事项

1. **字符串插值必须在字符串字面量内部使用**
   - ✅ `"Hello \(name)"`
   - ❌ `string\(variable)`

2. **变量必须先声明后使用**
   ```swift
   let name = "Swift"  // 先声明
   print(name)         // 后使用
   ```

3. **常量（let）不能重新赋值**
   ```swift
   let name = "Swift"
   name = "Python"  // ❌ 错误：常量不能重新赋值
   
   var language = "Swift"
   language = "Python"  // ✅ 正确：变量可以重新赋值
   ```

4. **支持 Unicode 字符**
   ```swift
   let π = 3.14159
   let 你好 = "Hello"
   let 🐶 = "dog"
   ```

## 当前限制

- 字符串插值目前只能解析，求值功能还在完善中
- 不支持复杂表达式
- 不支持函数定义
- 不支持控制流语句（if/else, for, while）