// 最基本的错误处理测试

// 定义简单的错误类型
enum SimpleError {
    case basic
    case withCode(code: Int)
}

// 简单的抛出错误的函数
func testFunction(shouldFail: Bool) throws -> String {
    if shouldFail {
        throw SimpleError.basic
    }
    return "成功"
}

// 基本的do-catch测试
print("开始测试")

do {
    let result = try testFunction(shouldFail: false)
    print("成功: " + result)
} catch {
    print("捕获到错误")
}

do {
    let result = try testFunction(shouldFail: true)
    print("不应该到达这里")
} catch {
    print("正确捕获到错误")
}

print("测试完成")