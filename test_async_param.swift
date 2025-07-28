// 测试异步函数参数绑定

func simpleAsync(userId: Int) async -> String {
    print("Inside simpleAsync, userId: \(userId)")
    return "Result for user \(userId)"
}

func testAsync() async -> String {
    let result = await simpleAsync(userId: 42)
    return result
}

@main
func main() {
    print("Testing async parameter binding...")
    
    let result = await testAsync()
    print("Final result: \(result)")
    
    print("Test completed.")
}

main()