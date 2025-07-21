// 简化的错误处理测试，避免使用switch语句

// 1. 定义错误类型
enum NetworkError {
    case timeout
    case connectionLost
    case invalidURL
    case serverError(code: Int)
}

enum FileError {
    case notFound
    case permissionDenied
    case corruptedData
}

enum ValidationError {
    case emptyInput
    case invalidFormat
    case tooLong
}

// 2. 定义抛出错误的函数
func connectToServer(url: String) throws -> String {
    if url == "timeout.com" {
        throw NetworkError.timeout
    }
    if url == "error500.com" {
        throw NetworkError.serverError(code: 500)
    }
    if url == "invalid.url" {
        throw NetworkError.invalidURL
    }
    return "连接成功: " + url
}

func readFile(path: String) throws -> String {
    if path == "missing.txt" {
        throw FileError.notFound
    }
    if path == "protected.txt" {
        throw FileError.permissionDenied
    }
    return "文件内容: " + path
}

// 3. 基本错误处理测试
print("=== 基本错误处理测试 ===")

// 测试成功情况
do {
    let result = try connectToServer(url: "example.com")
    print(result)
} catch NetworkError.timeout {
    print("捕获到超时错误")
} catch NetworkError.serverError(let code) {
    print("服务器错误，代码: " + String(code))
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

// 测试服务器错误
do {
    let result = try connectToServer(url: "error500.com")
    print("不应该到达这里")
} catch NetworkError.serverError(let code) {
    print("捕获到服务器错误，代码: " + String(code))
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

// 测试网络错误传播
do {
    let result = try processData(url: "timeout.com", filePath: "data.txt")
    print("不应该到达这里")
} catch NetworkError.timeout {
    print("网络错误成功传播")
} catch {
    print("错误传播失败")
}

// 测试文件错误传播
do {
    let result = try processData(url: "api.com", filePath: "protected.txt")
    print("不应该到达这里")
} catch FileError.permissionDenied {
    print("文件错误成功传播")
} catch {
    print("错误传播失败")
}

// 7. Guard 语句测试
print("\n=== Guard 语句测试 ===")

func processOptionalData(data: String?) -> String {
    guard let validData = data else {
        print("数据为空，提前返回")
        return "失败"
    }
    
    guard validData.count > 0 else {
        print("数据长度为0，提前返回")
        return "失败"
    }
    
    return "处理成功: " + validData
}

let guardResult1 = processOptionalData(data: "有效数据")
print(guardResult1)

let guardResult2 = processOptionalData(data: nil)
print(guardResult2)

let guardResult3 = processOptionalData(data: "")
print(guardResult3)

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

// 9. 嵌套错误处理测试
print("\n=== 嵌套错误处理测试 ===")

func nestedErrorHandling() {
    do {
        print("外层 do 开始")
        
        do {
            print("内层 do 开始")
            try connectToServer(url: "timeout.com")
            print("内层不应该到达这里")
        } catch NetworkError.timeout {
            print("内层捕获超时错误")
            throw ValidationError.invalidFormat  // 重新抛出不同类型的错误
        }
        
        print("外层不应该到达这里")
    } catch ValidationError.invalidFormat {
        print("外层捕获验证错误")
    } catch {
        print("外层捕获其他错误")
    }
}

nestedErrorHandling()

print("\n=== 错误处理测试完成 ===")