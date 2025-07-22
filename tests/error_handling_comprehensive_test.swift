// 错误处理综合测试
// 测试 Error 协议、throw/try/catch 语法、Result 类型和错误传播

// // 1. 自定义错误类型定义
// enum NetworkError: Error {
//     case timeout
//     case connectionLost
//     case invalidURL
//     case serverError(code: Int)
// }

// enum FileError: Error {
//     case notFound
//     case permissionDenied
//     case corrupted
//     case diskFull
// }

// enum ValidationError: Error {
//     case emptyInput
//     case invalidFormat
//     case tooLong
//     case tooShort
// }

// // 2. 抛出错误的函数测试
// func connectToServer(url: String) throws -> String {
//     if url == "" {
//         throw NetworkError.invalidURL
//     }
//     if url == "timeout.com" {
//         throw NetworkError.timeout
//     }
//     if url == "error500.com" {
//         throw NetworkError.serverError(code: 500)
//     }
//     return "Connected to " + url
// }

// func readFile(path: String) throws -> String {
//     if path == "" {
//         throw FileError.notFound
//     }
//     if path == "protected.txt" {
//         throw FileError.permissionDenied
//     }
//     if path == "corrupted.txt" {
//         throw FileError.corrupted
//     }
//     return "File content from " + path
// }

// func validateInput(text: String) throws -> String {
//     if text == "" {
//         throw ValidationError.emptyInput
//     }
//     // 简化：假设短字符串检查
//     if text == "a" || text == "ab" {
//         throw ValidationError.tooShort
//     }
//     // 简化：跳过长度检查
// //     if false {
// //         throw ValidationError.tooLong
// //     }
// //     return "Valid: " + text
// // }

// // // 3. Do-Catch 语句测试
// // print("=== Do-Catch 测试 ===")

// // // 测试网络错误处理
// // do {
// //     let result = try connectToServer(url: "example.com")
// //     print("成功: " + result)
// // } catch NetworkError.timeout {
// //     print("网络超时")
// // } catch NetworkError.connectionLost {
// //     print("连接丢失")
// // } catch NetworkError.invalidURL {
// //     print("无效URL")
// // } catch NetworkError.serverError(let code) {
// //     print("服务器错误，代码: \(code)")
// // } catch {
// //     print("未知网络错误")
// // }

// // // 测试超时错误
// // do {
// //     let result = try connectToServer(url: "timeout.com")
// //     print("不应该到达这里")
// // } catch NetworkError.timeout {
// //     print("捕获到超时错误")
// // } catch {
// //     print("其他错误: \(error)")
// // }

// // // 测试服务器错误
// // do {
// //     let result = try connectToServer(url: "error500.com")
// //     print("不应该到达这里")
// // } catch NetworkError.serverError(let code) {
// //     print("捕获到服务器错误，代码: \(code)")
// // } catch {
// //     print("其他错误")
// // }

// // 4. Try? 可选试用测试
// print("\n=== Try? 测试 ===")

// let result1 = try? connectToServer(url: "example.com")
// if let success = result1 {
//     print("Try? 成功: " + success)
// } else {
//     print("Try? 失败")
// }

// let result2 = try? connectToServer(url: "timeout.com")
// if let success = result2 {
//     print("不应该到达这里")
// } else {
//     print("Try? 正确返回 nil")
// }

// // 5. Try! 强制试用测试
// print("\n=== Try! 测试 ===")

// let result3 = try! connectToServer(url: "example.com")
// print("Try! 成功: " + result3)

// // 注意：下面的代码会导致运行时错误，在实际测试中应该注释掉
// // let result4 = try! connectToServer(url: "timeout.com")  // 这会崩溃

// // 6. 错误传播测试
// print("\n=== 错误传播测试 ===")

// // func processData(url: String, filePath: String) throws -> String {
// //     let connection = try connectToServer(url: url)
// //     let fileContent = try readFile(path: filePath)
// //     return connection + " + " + fileContent
// // }

// // do {
// //     let result = try processData(url: "api.com", filePath: "data.txt")
// //     print("处理成功: " + result)
// // } catch NetworkError.timeout {
// //     print("网络超时导致处理失败")
// // } catch FileError.notFound {
// //     print("文件未找到导致处理失败")
// // } catch {
// //     print("其他错误导致处理失败")
// // }

// // // // 测试网络错误传播
// // // do {
// // //     let result = try processData(url: "timeout.com", filePath: "data.txt")
// // //     print("不应该到达这里")
// // // } catch NetworkError.timeout {
// // //     print("网络错误成功传播")
// // // } catch {
// // //     print("错误传播失败")
// // // }

// // // 测试文件错误传播
// // do {
// //     let result = try processData(url: "api.com", filePath: "protected.txt")
// //     print("不应该到达这里")
// // } catch FileError.permissionDenied {
// //     print("文件错误成功传播")
// // } catch {
// //     print("错误传播失败")
// // }

// // // 7. Guard 语句测试
// // print("\n=== Guard 语句测试 ===")

// // func processOptionalData(data: String?) -> String {
// //     guard let validData = data else {
// //         print("数据为空，提前返回")
// //         return "失败"
// //     }
    
// //     guard validData != "" else {
// //         print("数据长度为0，提前返回")
// //         return "失败"
// //     }
    
// //     return "处理成功: " + validData
// // }

// // let guardResult1 = processOptionalData(data: "有效数据")
// // print(guardResult1)

// // let guardResult2 = processOptionalData(data: nil)
// // print(guardResult2)

// // let guardResult3 = processOptionalData(data: "")
// // print(guardResult3)

// // // 8. Defer 语句测试
// // print("\n=== Defer 语句测试 ===")

// // func testDefer() {
// //     print("函数开始")
    
// //     defer {
// //         print("Defer 1: 第一个延迟执行")
// //     }
    
// //     defer {
// //         print("Defer 2: 第二个延迟执行")
// //     }
    
// //     print("函数中间")
    
// //     defer {
// //         print("Defer 3: 第三个延迟执行")
// //     }
    
// //     print("函数结束")
// // }

// // testDefer()

// // // 9. 嵌套错误处理测试
// // print("\n=== 嵌套错误处理测试 ===")

// func nestedErrorHandling() {
//     do {
//         print("外层 do 开始")
        
//         do {
//             print("内层 do 开始")
//             // try connectToServer(url: "timeout.com")
//             print("内层不应该到达这里")
//         } catch NetworkError.timeout {
//             print("内层捕获超时错误")
//             throw ValidationError.invalidFormat  // 重新抛出不同类型的错误
//         }
        
//         print("外层不应该到达这里")
//     } catch ValidationError.invalidFormat {
//         print("外层捕获验证错误")
//     } catch {
//         print("外层捕获其他错误")
//     }
// }

// nestedErrorHandling()

// // 10. 多种错误类型混合测试
// print("\n=== 多种错误类型混合测试 ===")

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

for i in 1...4 {
    do {
        try mixedErrorTest(scenario: i)
        print("场景 \(i): 成功")
    } catch NetworkError.timeout {
        print("场景 \(i): 网络超时")
    } catch FileError.notFound {
        print("场景 \(i): 文件未找到")
    } catch ValidationError.emptyInput {
        print("场景 \(i): 输入为空")
    } catch {
        print("场景 \(i): 未知错误")
    }
}



print("\n=== 错误处理测试完成 ===")