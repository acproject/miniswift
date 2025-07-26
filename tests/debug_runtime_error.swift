// 调试运行时错误 - 逐步测试可能导致断言失败的功能

print("=== 调试运行时错误测试 ===")

// 测试1：基本print
print("Test 1: Basic print")

// 测试2：简单变量
let x = 10
print("Test 2: Variable x = 10")

// 测试3：简单字符串
let str = "Hello"
print("Test 3: String")
print(str)

// 测试4：简单字符串插值
print("Test 4: Simple interpolation")
print("Value: \(x)")

print("=== 如果到这里没有崩溃，继续下一步 ===")