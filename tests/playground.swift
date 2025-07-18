// MiniSwift Playground - 用于测试各种语法功能

// 基本变量声明
let name = "Swift"
var age = 25

// 字符串插值测试
let greeting = "Hello \(name)!"
let info = "I am \(age) years old"

// 打印测试
print("=== 基本功能测试 ===")
print(name)
print(age)
print(greeting)
print(info)

// 数学运算
let a = 10
let b = 5
let sum = a + b
let product = a * b

print("\n=== 数学运算测试 ===")
print("a = \(a), b = \(b)")
print("sum = \(sum)")
print("product = \(product)")

// 字符串边界情况
let emptyString = ""
let stringWithSpaces = "  hello world  "
let specialChars = "Hello, 世界! @#$%"

print("\n=== 字符串边界测试 ===")
print("Empty: '\(emptyString)'")
print("Spaces: '\(stringWithSpaces)'")
print("Special: '\(specialChars)'")

// 嵌套插值测试（如果支持）
let x = 3
let y = 4
let nested = "Result: \(x + y) = \(x) + \(y)"
print("\n=== 嵌套插值测试 ===")
print(nested)

// 多行测试
print("\n=== 多行测试 ===")
print("Line 1")
print("Line 2")
print("Line 3")

print("\n=== 测试完成 ===")