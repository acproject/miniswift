// 基础Switch和错误处理测试
print("=== 基础Switch和错误处理测试 ===")

// 1. 基本Switch测试
print("\n=== 基本Switch测试 ===")

let number = 2
switch number {
case 1:
    print("数字是1")
case 2:
    print("数字是2")
case 3:
    print("数字是3")
default:
    print("其他数字")
}

let text = "hello"
switch text {
case "hello":
    print("问候语")
case "goodbye":
    print("告别语")
default:
    print("其他文本")
}

// 2. 基本错误处理测试（不使用枚举）
print("\n=== 基本错误处理测试 ===")

func simpleThrowFunction(shouldThrow: Bool) throws -> String {
    if shouldThrow {
        throw "简单错误"
    }
    return "成功"
}

do {
    let result = try simpleThrowFunction(shouldThrow: false)
    print("成功: " + result)
} catch {
    print("捕获到错误")
}

do {
    let result = try simpleThrowFunction(shouldThrow: true)
    print("不应该到达这里")
} catch {
    print("捕获到错误")
}

// 3. Try? 测试
print("\n=== Try? 测试 ===")

let result1 = try? simpleThrowFunction(shouldThrow: false)
if let success = result1 {
    print("Try? 成功: " + success)
} else {
    print("Try? 失败")
}

let result2 = try? simpleThrowFunction(shouldThrow: true)
if let success = result2 {
    print("不应该到达这里")
} else {
    print("Try? 正确返回nil")
}

// 4. Try! 测试
print("\n=== Try! 测试 ===")

let result3 = try! simpleThrowFunction(shouldThrow: false)
print("Try! 成功: " + result3)

// 5. Switch和错误处理结合
print("\n=== Switch和错误处理结合 ===")

func processScenario(scenario: Int) throws -> String {
    switch scenario {
    case 1:
        throw "场景1错误"
    case 2:
        throw "场景2错误"
    case 3:
        return "场景3成功"
    default:
        return "默认场景成功"
    }
}

do {
    let result = try processScenario(scenario: 1)
    print("场景1成功: " + result)
} catch {
    print("场景1失败")
}

do {
    let result = try processScenario(scenario: 3)
    print("场景3成功: " + result)
} catch {
    print("场景3失败")
}

do {
    let result = try processScenario(scenario: 5)
    print("场景5成功: " + result)
} catch {
    print("场景5失败")
}

print("\n=== 测试完成 ===")