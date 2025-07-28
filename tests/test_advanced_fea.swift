// 复杂的Swift程序测试 - 编译为可执行文件

// 数据结构定义
struct User {
    let id: Int
    let name: String
    var age: Int
    
    func getInfo() -> String {
        return "User(id: \(id), name: \(name), age: \(age))"
    }
    
    mutating func incrementAge() {
        age = age + 1
    }
}

// 枚举定义
enum TaskStatus {
    case pending
    case running
    case completed
    case failed
}

// 业务逻辑结构
struct TaskProcessor {
    let processorId: String
    
    func processTask(taskId: Int, data: String) -> String {
        if data.isEmpty {
            return "Error: Invalid input"
        }
        
        let result = "Processor[\(processorId)] processed task \(taskId) with data: \(data)"
        return result
    }
    
    func getStatus(taskId: Int) -> TaskStatus {
        if taskId < 0 {
            return TaskStatus.failed
        } else if taskId < 10 {
            return TaskStatus.completed
        } else if taskId < 20 {
            return TaskStatus.running
        } else {
            return TaskStatus.pending
        }
    }
}

// 数学计算函数
func fibonacci(n: Int) -> Int {
    if n <= 1 {
        return n
    }
    return fibonacci(n: n - 1) + fibonacci(n: n - 2)
}

// 条件处理函数
func categorizeAge(age: Int) -> String {
    if age < 18 {
        return "未成年"
    } else if age < 60 {
        return "成年人"
    } else {
        return "老年人"
    }
}

// 异步函数测试
func fetchUserData(userId: Int) async -> User {
    return User(id: userId, name: "User\(userId)", age: 25 + userId)
}

func processUserAsync(userId: Int) async -> String {
    let user = await fetchUserData(userId: userId)
    let processor = TaskProcessor(processorId: "ASYNC_PROC")
    
    let result = processor.processTask(taskId: userId, data: user.name)
    return "异步处理成功: \(result)"
}

// 主程序入口函数
@main
func main() {
    print("=== MiniSwift 复杂程序测试 ===")
    print("")
    
    // 测试1: 基本数据结构
    print("1. 测试基本数据结构:")
    var user1 = User(id: 1, name: "张三", age: 25)
    print("   创建用户: \(user1.getInfo())")
    
    user1.incrementAge()
    print("   年龄增加后: \(user1.getInfo())")
    print("")
    
    // 测试2: 枚举和业务逻辑
    print("2. 测试枚举和业务逻辑:")
    let processor = TaskProcessor(processorId: "MAIN_PROC")
    
    let result1 = processor.processTask(taskId: 1, data: "测试数据")
    print("   处理成功: \(result1)")
    
    let status = processor.getStatus(taskId: 5)
    print("   任务状态: \(status)")
    print("")
    
    // 测试3: 递归函数
    print("3. 测试递归函数:")
    let fibResult = fibonacci(n: 8)
    print("   斐波那契数列第8项: \(fibResult)")
    print("")
    
    // 测试4: 条件分支
    print("4. 测试条件分支:")
    let age1 = 15
    let age2 = 25
    let age3 = 65
    
    let category1 = categorizeAge(age: age1)
    let category2 = categorizeAge(age: age2)
    let category3 = categorizeAge(age: age3)
    
    print("   年龄 \(age1): \(category1)")
    print("   年龄 \(age2): \(category2)")
    print("   年龄 \(age3): \(category3)")
    print("")
    
    // 测试5: 异步处理
    print("5. 测试异步处理:")
    let asyncResult = await processUserAsync(userId: 10)
    print("   \(asyncResult)")
    print("")
    
    // 测试6: 循环和计算
    print("6. 测试循环和计算:")
    var sum = 0
    for i in 1...10 {
        sum = sum + i
    }
    print("   1到10的和: \(sum)")
    
    var factorial = 1
    for i in 1...5 {
        factorial = factorial * i
    }
    print("   5的阶乘: \(factorial)")
    print("")
    
    // 测试7: 多个用户处理
    print("7. 测试多个用户处理:")
    let user2 = User(id: 2, name: "Bob", age: 35)
    let user3 = User(id: 3, name: "Charlie", age: 42)
    
    print("   用户2: \(user2.getInfo())")
    print("   用户3: \(user3.getInfo())")
    
    let processor2 = TaskProcessor(processorId: "USER_PROC")
    let result2 = processor2.processTask(taskId: user2.id, data: user2.name)
    let result3 = processor2.processTask(taskId: user3.id, data: user3.name)
    
    print("   处理结果2: \(result2)")
    print("   处理结果3: \(result3)")
    print("")
    
    // 测试8: 综合应用
    print("8. 综合应用测试:")
    let finalUser = User(id: 999, name: "TestUser", age: 30)
    let finalProcessor = TaskProcessor(processorId: "FINAL_TEST")
    
    let finalResult = finalProcessor.processTask(
        taskId: finalUser.id, 
        data: finalUser.name
    )
    print("   最终测试结果: \(finalResult)")
    
    let userCategory = categorizeAge(age: finalUser.age)
    print("   用户分类: \(userCategory)")
    
    let fibValue = fibonacci(n: 6)
    print("   斐波那契第6项: \(fibValue)")
    
    print("")
    print("=== 复杂程序测试完成 ===")
    print("成功测试的特性:")
    print("- 结构体和方法")
    print("- 枚举和模式匹配")
    print("- 递归函数")
    print("- 异步编程")
    print("- 循环和条件")
    print("- 复杂业务逻辑")
}

// 全局常量
let PROGRAM_VERSION = "1.0.0"
let MAX_USERS = 1000

// 全局函数
func getVersion() -> String {
    return "MiniSwift Complex Test v\(PROGRAM_VERSION)"
}

main()