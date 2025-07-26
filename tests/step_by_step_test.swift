// 逐步测试 - 分步验证MiniSwift功能

// 1. 简单函数测试
func simpleAdd(a: Int, b: Int) -> Int {
    return a + b
}

func testSimpleFunctions() {
    print("=== 测试简单函数 ===")
    let result = simpleAdd(a: 5, b: 3)
    print("5 + 3 = \(result)")
}

// 2. 数组测试
func testArrays() {
    print("\n=== 测试数组 ===")
    let numbers = [1, 2, 3, 4, 5]
    print("Numbers: \(numbers)")
    print("First number: \(numbers[0])")
    print("Array count: \(numbers.count)")
}

// 3. 循环测试
func testLoops() {
    print("\n=== 测试循环 ===")
    print("For loop with range:")
    for i in 1...3 {
        print("i = \(i)")
    }
    
    print("For loop with array:")
    let items = ["apple", "banana", "cherry"]
    for item in items {
        print("Item: \(item)")
    }
}

// 4. 条件语句测试
func testConditions() {
    print("\n=== 测试条件语句 ===")
    let x = 10
    
    if x > 5 {
        print("x is greater than 5")
    }
    
    if x < 20 {
        print("x is less than 20")
    }
    
    let y = 3
    if y == 3 {
        print("y equals 3")
    } else {
        print("y does not equal 3")
    }
}

// 5. Switch语句测试
func testSwitch() {
    print("\n=== 测试Switch语句 ===")
    let number = 2
    switch number {
    case 1:
        print("One")
    case 2:
        print("Two")
    case 3:
        print("Three")
    default:
        print("Other")
    }
}

// 6. 结构体测试
struct SimplePoint {
    var x: Int
    var y: Int
    
    func describe() -> String {
        return "Point at (\(x), \(y))"
    }
}

func testStructs() {
    print("\n=== 测试结构体 ===")
    let point = SimplePoint(x: 10, y: 20)
    print(point.describe())
    print("X coordinate: \(point.x)")
    print("Y coordinate: \(point.y)")
}

// 7. 类测试
class SimpleCounter {
    var count: Int
    
    init(startValue: Int) {
        self.count = startValue
    }
    
    func increment() {
        count = count + 1
    }
    
    func getValue() -> Int {
        return count
    }
}

func testClasses() {
    print("\n=== 测试类 ===")
    let counter = SimpleCounter(startValue: 0)
    print("Initial count: \(counter.getValue())")
    
    counter.increment()
    print("After increment: \(counter.getValue())")
    
    counter.increment()
    print("After second increment: \(counter.getValue())")
}

// 8. 字典测试
func testDictionaries() {
    print("\n=== 测试字典 ===")
    var ages: [String: Int] = [:]
    ages["Alice"] = 25
    ages["Bob"] = 30
    
    print("Ages dictionary: \(ages)")
    print("Alice's age: \(ages[\"Alice\"])")
}

// 9. 递归函数测试
func simpleFactorial(n: Int) -> Int {
    if n <= 1 {
        return 1
    }
    return n * simpleFactorial(n: n - 1)
}

func testRecursion() {
    print("\n=== 测试递归函数 ===")
    let fact5 = simpleFactorial(n: 5)
    print("Factorial of 5: \(fact5)")
}

// 10. 字符串插值测试
func testStringInterpolation() {
    print("\n=== 测试字符串插值 ===")
    let name = "World"
    let greeting = "Hello, \(name)!"
    print(greeting)
    
    let x = 42
    let message = "The answer is \(x)"
    print(message)
}

// 主测试函数
func runStepByStepTest() {
    print("开始逐步测试MiniSwift功能...\n")
    
    testSimpleFunctions()
    testArrays()
    testLoops()
    testConditions()
    testSwitch()
    testStructs()
    testClasses()
    testDictionaries()
    testRecursion()
    testStringInterpolation()
    
    print("\n=== 所有测试完成 ===")
}

// 运行测试
runStepByStepTest()