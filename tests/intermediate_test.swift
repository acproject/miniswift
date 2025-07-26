// 中等复杂度测试 - 逐步增加功能

print("=== MiniSwift 中等复杂度测试 ===")

// 1. 基础数据类型和运算
print("\n--- 基础数据类型测试 ---")
let intValue = 42
let doubleValue = 3.14
let stringValue = "Hello"
let boolValue = true

print("Int: \(intValue)")
print("Double: \(doubleValue)")
print("String: \(stringValue)")
print("Bool: \(boolValue)")

// 2. 数组操作
print("\n--- 数组操作测试 ---")
let numbers = [1, 2, 3, 4, 5]
print("Original array: \(numbers)")
print("First element: \(numbers[0])")
print("Last element: \(numbers[4])")
print("Array count: \(numbers.count)")

// 3. 循环和条件
print("\n--- 循环和条件测试 ---")
for num in numbers {
    if num % 2 == 0 {
        print("\(num) is even")
    } else {
        print("\(num) is odd")
    }
}

// 4. Switch语句
print("\n--- Switch语句测试 ---")
for i in 1...3 {
    switch i {
    case 1:
        print("First")
    case 2:
        print("Second")
    case 3:
        print("Third")
    default:
        print("Other")
    }
}

// 5. 函数定义和调用
print("\n--- 函数测试 ---")
func multiply(a: Int, b: Int) -> Int {
    return a * b
}

func greet(name: String) -> String {
    return "Hello, \(name)!"
}

let product = multiply(a: 6, b: 7)
print("6 * 7 = \(product)")

let greeting = greet(name: "Swift")
print(greeting)

// 6. 结构体测试
print("\n--- 结构体测试 ---")
struct Point {
    var x: Int
    var y: Int
    
    func describe() -> String {
        return "Point(\(x), \(y))"
    }
    
    func distanceFromOrigin() -> Double {
        let dx = Double(x)
        let dy = Double(y)
        return sqrt(dx * dx + dy * dy)
    }
}

let point1 = Point(x: 3, y: 4)
print(point1.describe())
print("Distance from origin: \(point1.distanceFromOrigin())")

let point2 = Point(x: 0, y: 0)
print(point2.describe())

// 7. 类测试
print("\n--- 类测试 ---")
class Counter {
    var value: Int
    
    init(startValue: Int) {
        self.value = startValue
    }
    
    func increment() {
        value = value + 1
    }
    
    func decrement() {
        value = value - 1
    }
    
    func reset() {
        value = 0
    }
    
    func getValue() -> Int {
        return value
    }
}

let counter = Counter(startValue: 10)
print("Initial value: \(counter.getValue())")

counter.increment()
print("After increment: \(counter.getValue())")

counter.decrement()
print("After decrement: \(counter.getValue())")

counter.reset()
print("After reset: \(counter.getValue())")

// 8. 字典测试
print("\n--- 字典测试 ---")
var scores: [String: Int] = [:]
scores["Alice"] = 95
scores["Bob"] = 87
scores["Charlie"] = 92

print("Scores: \(scores)")
print("Alice's score: \(scores[\"Alice\"])")
print("Bob's score: \(scores[\"Bob\"])")

// 9. 嵌套数据结构
print("\n--- 嵌套数据结构测试 ---")
let matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
print("Matrix:")
for row in matrix {
    print(row)
}

print("Element at [1][1]: \(matrix[1][1])")

// 10. 递归函数
print("\n--- 递归函数测试 ---")
func factorial(n: Int) -> Int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n: n - 1)
}

for i in 1...5 {
    print("\(i)! = \(factorial(n: i))")
}

// 11. 字符串操作
print("\n--- 字符串操作测试 ---")
let firstName = "John"
let lastName = "Doe"
let fullName = firstName + " " + lastName
print("Full name: \(fullName)")

let message = "The result is \(product)"
print(message)

print("\n=== 中等复杂度测试完成 ===")