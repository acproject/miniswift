// 基础错误处理测试
// 逐步测试各个功能

print("=== 开始基础错误处理测试 ===")

// 1. 简单的错误枚举定义
enum SimpleError: Error {
    case basic
    case advanced
}

print("错误类型定义成功")

// 2. 简单的抛出错误函数
func throwBasicError() throws {
    throw SimpleError.basic
}

print("抛出错误函数定义成功")

// 3. 基础 do-catch 测试
print("\n=== Do-Catch 测试 ===")

do {
    try throwBasicError()
    print("不应该到达这里")
} catch SimpleError.basic {
    print("成功捕获基础错误")
} catch {
    print("捕获其他错误")
}

// 4. 简单的返回值函数
func returnValue() throws -> String {
    return "成功返回"
}

print("\n=== 返回值测试 ===")

do {
    let result = try returnValue()
    print("结果: " + result)
} catch {
    print("意外错误")
}

print("\n=== 基础错误处理测试完成 ===")