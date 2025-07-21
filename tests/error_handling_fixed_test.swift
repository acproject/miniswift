// 错误处理综合测试 - 修复版本
print("=== 错误处理综合测试 ===")

// 1. 定义错误类型
enum NetworkError {
    case timeout
    case connectionFailed
    case serverError(Int)
}

enum FileError {
    case notFound
    case permissionDenied
    case corruptedData
}

enum ValidationError {
    case emptyInput
    case invalidFormat
    case outOfRange
}

// 2. 抛出错误的函数
func connectToServer(url: String) throws -> String {
    if url == "timeout.com" {
        throw NetworkError.timeout
    } else if url == "error500.com" {
        throw NetworkError.serverError(500)
    } else if url == "failed.com" {
        throw NetworkError.connectionFailed
    }
    return "连接成功: " + url
}

func readFile(path: String) throws -> String {
    if path == "missing.txt" {
        throw FileError.notFound
    } else if path == "protected.txt" {
        throw FileError.permissionDenied
    } else if path == "corrupted.txt" {
        throw FileError.corruptedData
    }
    return "文件内容: " + path
}

func validateInput(input: String) throws -> String {
    if input == "" {
        throw ValidationError.emptyInput
    } else if input == "invalid" {
        throw ValidationError.invalidFormat
    } else if input == "outofrange" {
        throw ValidationError.outOfRange
    }
    return "验证通过: " + input
}

// 3. 基本错误处理测试
print("\n=== 基本错误处理测试 ===")

// 测试成功情况
do {
    let result = try connectToServer(url: "example.com")
    print("成功: " + result)
} catch NetworkError.timeout {
    print("网络超时")
} catch NetworkError.connectionFailed {
    print("连接失败")
} catch NetworkError.serverError(let code) {
    print("服务器错误")
} catch {
    print("未知网络错误")
}

// 测试超时错误
do {
    let result = try connectToServer(url: "timeout.com")
    print("不应该到达这里")
} catch NetworkError.timeout {
    print("捕获到超时错误")
} catch {
    print("其他错误")
}

// 4. Try? 可选试用测试
print("\n=== Try? 测试 ===")

let result1 = try? connectToServer(url: "example.com")
if let success = result1 {
    print("Try? 成功: " + success)
} else {
    print("Try? 失败")
}

let result2 = try? connectToServer(url: "timeout.com")
if let success = result2 {
    print("不应该到达这里")
} else {
    print("Try? 正确返回 nil")
}

// 5. Try! 强制试用测试
print("\n=== Try! 测试 ===")

let result3 = try! connectToServer(url: "example.com")
print("Try! 成功: " + result3)

// 6. 错误传播测试
print("\n=== 错误传播测试 ===")

func processData(url: String, filePath: String) throws -> String {
    let connection = try connectToServer(url: url)
    let fileContent = try readFile(path: filePath)
    return connection + " + " + fileContent
}

do {
    let result = try processData(url: "api.com", filePath: "data.txt")
    print("处理成功: " + result)
} catch NetworkError.timeout {
    print("网络超时导致处理失败")
} catch FileError.notFound {
    print("文件未找到导致处理失败")
} catch {
    print("其他错误导致处理失败")
}

// 7. Guard 语句测试
print("\n=== Guard 语句测试 ===")

func processOptionalData(data: String?) -> String {
    guard let validData = data else {
        print("数据为空，提前返回")
        return "失败"
    }
    
    return "处理成功: " + validData
}

let guardResult1 = processOptionalData(data: "有效数据")
print(guardResult1)

let guardResult2 = processOptionalData(data: nil)
print(guardResult2)

// 8. Defer 语句测试
print("\n=== Defer 语句测试 ===")

func testDefer() {
    print("函数开始")
    
    defer {
        print("Defer 1: 第一个延迟执行")
    }
    
    defer {
        print("Defer 2: 第二个延迟执行")
    }
    
    print("函数中间")
    
    defer {
        print("Defer 3: 第三个延迟执行")
    }
    
    print("函数结束")
}

testDefer()

// 9. Switch 语句错误处理测试
print("\n=== Switch 语句错误处理测试 ===")

func mixedErrorTest(scenario: Int) throws {
    switch scenario {
    case 1:
        throw NetworkError.timeout
    case 2:
        throw FileError.notFound
    case 3:
        throw ValidationError.emptyInput
    default:
        print("正常执行")
    }
}

// 测试场景 1
do {
    try mixedErrorTest(scenario: 1)
    print("场景 1: 成功")
} catch NetworkError.timeout {
    print("场景 1: 网络超时")
} catch FileError.notFound {
    print("场景 1: 文件未找到")
} catch ValidationError.emptyInput {
    print("场景 1: 输入为空")
} catch {
    print("场景 1: 未知错误")
}

// 测试场景 2
do {
    try mixedErrorTest(scenario: 2)
    print("场景 2: 成功")
} catch NetworkError.timeout {
    print("场景 2: 网络超时")
} catch FileError.notFound {
    print("场景 2: 文件未找到")
} catch ValidationError.emptyInput {
    print("场景 2: 输入为空")
} catch {
    print("场景 2: 未知错误")
}

// 测试场景 3
do {
    try mixedErrorTest(scenario: 3)
    print("场景 3: 成功")
} catch NetworkError.timeout {
    print("场景 3: 网络超时")
} catch FileError.notFound {
    print("场景 3: 文件未找到")
} catch ValidationError.emptyInput {
    print("场景 3: 输入为空")
} catch {
    print("场景 3: 未知错误")
}

// 测试场景 4 (正常情况)
do {
    try mixedErrorTest(scenario: 4)
    print("场景 4: 成功")
} catch NetworkError.timeout {
    print("场景 4: 网络超时")
} catch FileError.notFound {
    print("场景 4: 文件未找到")
} catch ValidationError.emptyInput {
    print("场景 4: 输入为空")
} catch {
    print("场景 4: 未知错误")
}

print("\n=== 错误处理测试完成 ===")