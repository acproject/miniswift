// 测试带标签的函数调用

// 带标签参数的函数
func testFunction(userId: Int, name: String) -> String {
    return "User ID: \(userId), Name: \(name)"
}

// 异步函数测试
func asyncFunction(userId: Int) async -> String {
    let result = testFunction(userId: userId, name: "TestUser")
    return "Async result: \(result)"
}

@main
func main() {
    print("Testing labeled function calls...")
    
    // 直接调用
    let directResult = testFunction(userId: 42, name: "DirectUser")
    print("Direct call: \(directResult)")
    
    // 异步调用
    let asyncResult = await asyncFunction(userId: 100)
    print("\(asyncResult)")
    
    print("Test completed.")
}

main()