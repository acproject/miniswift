// 简单的错误处理和Switch测试
print("=== 简单错误处理和Switch测试 ===")

// 1. 定义简单错误类型
enum SimpleError {
    case error1
    case error2
}

// 2. 抛出错误的函数
func testFunction(value: Int) throws -> String {
    if value == 1 {
        throw SimpleError.error1
    } else if value == 2 {
        throw SimpleError.error2
    }
    return "成功"
}

// 3. 基本错误处理
print("\n=== 基本错误处理 ===")

do {
    let result = try testFunction(value: 0)
    print("成功: " + result)
} catch SimpleError.error1 {
    print("捕获到错误1")
} catch SimpleError.error2 {
    print("捕获到错误2")
} catch {
    print("其他错误")
}

do {
    let result = try testFunction(value: 1)
    print("不应该到达这里")
} catch SimpleError.error1 {
    print("捕获到错误1")
} catch SimpleError.error2 {
    print("捕获到错误2")
} catch {
    print("其他错误")
}

// 4. Switch语句测试
print("\n=== Switch语句测试 ===")

let testValue = 1
switch testValue {
case 1:
    print("值是1")
case 2:
    print("值是2")
default:
    print("其他值")
}

// 5. Switch和错误处理结合
print("\n=== Switch和错误处理结合 ===")

func processWithSwitch(scenario: Int) throws {
    switch scenario {
    case 1:
        throw SimpleError.error1
    case 2:
        throw SimpleError.error2
    default:
        print("正常处理")
    }
}

do {
    try processWithSwitch(scenario: 1)
    print("场景1成功")
} catch SimpleError.error1 {
    print("场景1: 捕获错误1")
} catch {
    print("场景1: 其他错误")
}

do {
    try processWithSwitch(scenario: 3)
    print("场景3成功")
} catch {
    print("场景3: 错误")
}

print("\n=== 测试完成 ===")