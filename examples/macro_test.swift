// 宏系统测试文件
// 测试基本宏声明和使用

// 1. 基本宏声明
macro stringify(value: String) -> String {
    return value
}

// 2. 外部宏声明
externalMacro URL(string: String) -> URL

// 3. 自由宏声明
@freestanding(expression)
macro assert(condition: Bool, message: String) {
    // 编译时展开为条件检查代码
}

// 4. 使用宏的示例
func testMacros() {
    // 使用基本宏
    let result = #stringify("42")
    print("Stringify result: " + result)
    
    // 使用自由宏
    #assert(true, "This should pass")
    
    // 宏展开表达式
    let expanded = #stringify("Hello, World!")
    print("Expanded: " + expanded)
}

// 5. 简单结构体
struct Person {
    var name: String
    var age: Int
}

// 测试函数
testMacros()