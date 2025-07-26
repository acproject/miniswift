// 完整的异步并发功能测试
// 测试 MiniSwift 编译器对异步编程的支持

// 基本异步函数
func fetchUserData(userId: Int) async -> String {
    return "User data for ID: \(userId)"
}

func fetchUserProfile(userId: Int) async -> String {
    return "Profile for user \(userId)"
}

// 异步数据处理
func processUserData(data: String) async -> String {
    return "Processed: \(data)"
}

// 复合异步操作
func getUserInfo(userId: Int) async -> String {
    let userData = await fetchUserData(userId: userId)
    let userProfile = await fetchUserProfile(userId: userId)
    let processedData = await processUserData(data: userData)
    
    return "Complete info: \(processedData), \(userProfile)"
}

// 异步错误处理
enum NetworkError: Error {
    case connectionFailed
    case timeout
    case invalidResponse
}

func riskyNetworkCall() async throws -> String {
    // 模拟网络调用
    return "Network call successful"
}

func safeNetworkCall() async -> String {
    do {
        let result = try await riskyNetworkCall()
        return "Success: \(result)"
    } catch {
        return "Error occurred during network call"
    }
}

// 异步计算函数
func calculateAsync(a: Int, b: Int) async -> Int {
    return a + b
}

func complexCalculation(numbers: Int) async -> String {
    let result1 = await calculateAsync(a: numbers, b: 10)
    let result2 = await calculateAsync(a: result1, b: 20)
    return "Calculation result: \(result2)"
}

// 主程序入口
@main
func main() async {
    print("=== MiniSwift 异步并发功能测试 ===")
    print("")
    
    // 测试1: 基本异步函数调用
    print("1. 测试基本异步函数:")
    let userData = await fetchUserData(userId: 123)
    print("   结果: \(userData)")
    print("")
    
    // 测试2: 异步函数链式调用
    print("2. 测试异步函数链:")
    let processedData = await processUserData(data: "sample data")
    print("   结果: \(processedData)")
    print("")
    
    // 测试3: 复合异步操作
    print("3. 测试复合异步操作:")
    let userInfo = await getUserInfo(userId: 456)
    print("   结果: \(userInfo)")
    print("")
    
    // 测试4: 异步错误处理
    print("4. 测试异步错误处理:")
    do {
        let networkResult = try await riskyNetworkCall()
        print("   成功: \(networkResult)")
    } catch {
        print("   捕获到错误")
    }
    print("")
    
    // 测试5: 安全的异步调用
    print("5. 测试安全异步调用:")
    let safeResult = await safeNetworkCall()
    print("   结果: \(safeResult)")
    print("")
    
    // 测试6: 异步计算
    print("6. 测试异步计算:")
    let calcResult = await complexCalculation(numbers: 5)
    print("   结果: \(calcResult)")
    print("")
    
    // 测试7: 多个异步调用
    print("7. 测试多个异步调用:")
    let data1 = await fetchUserData(userId: 1)
    let data2 = await fetchUserData(userId: 2)
    let data3 = await fetchUserData(userId: 3)
    print("   用户1: \(data1)")
    print("   用户2: \(data2)")
    print("   用户3: \(data3)")
    print("")
    
    print("=== 异步并发功能测试完成 ===")
    print("编译器成功支持了以下异步特性:")
    print("- async 函数定义")
    print("- await 关键字")
    print("- 异步函数链式调用")
    print("- 异步错误处理 (try/catch)")
    print("- 复合异步操作")
}

// 全局异步辅助函数
func globalAsyncHelper() async -> String {
    let result = await fetchUserData(userId: 999)
    return "Global helper: \(result)"
}

// 异步工具函数
func asyncUtility(message: String) async -> String {
    return "Utility processed: \(message)"
}