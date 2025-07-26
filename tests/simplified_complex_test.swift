// 简化的复杂测试 - 测试MiniSwift的核心功能

// 1. 基础类定义
class Animal {
    var name: String
    var age: Int
    
    init(name: String, age: Int) {
        self.name = name
        self.age = age
    }
    
    func speak() -> String {
        return "Animal makes a sound"
    }
    
    func getInfo() -> String {
        return "Name: \(name), Age: \(age)"
    }
}

class Dog {
    var name: String
    var age: Int
    var breed: String
    
    init(name: String, age: Int, breed: String) {
        self.name = name
        self.age = age
        self.breed = breed
    }
    
    func speak() -> String {
        return "\(name) barks: Woof!"
    }
    
    func wagTail() -> String {
        return "\(name) is wagging tail happily!"
    }
    
    func getInfo() -> String {
        return "Name: \(name), Age: \(age), Breed: \(breed)"
    }
}

// 2. 结构体和方法
struct Point {
    var x: Double
    var y: Double
    
    func distance(to other: Point) -> Double {
        let dx = x - other.x
        let dy = y - other.y
        return sqrt(dx * dx + dy * dy)
    }
}

// 3. 数组处理函数
func doubleNumbers(_ numbers: [Int]) -> [Int] {
    var result: [Int] = []
    for num in numbers {
        result.append(num * 2)
    }
    return result
}

func filterEven(_ numbers: [Int]) -> [Int] {
    var result: [Int] = []
    for num in numbers {
        if num % 2 == 0 {
            result.append(num)
        }
    }
    return result
}

// 4. 字典操作函数
func countItems(_ items: [String]) -> [String: Int] {
    var counts: [String: Int] = [:]
    for item in items {
        if let currentCount = counts[item] {
            counts[item] = currentCount + 1
        } else {
            counts[item] = 1
        }
    }
    return counts
}

// 5. 递归函数
func factorial(_ n: Int) -> Int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}

func fibonacci(_ n: Int) -> Int {
    if n <= 1 {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}

// 6. 数学计算函数
func calculateSum(_ numbers: [Int]) -> Int {
    var sum = 0
    for num in numbers {
        sum += num
    }
    return sum
}

func findMax(_ numbers: [Int]) -> Int {
    if numbers.count == 0 {
        return 0
    }
    
    var max = numbers[0]
    for i in 1..<numbers.count {
        if numbers[i] > max {
            max = numbers[i]
        }
    }
    return max
}

// 7. 字符串处理函数
func reverseString(_ str: String) -> String {
    var result = ""
    let chars = Array(str)
    for i in (0..<chars.count).reversed() {
        result += String(chars[i])
    }
    return result
}

// 8. 主测试函数
func runSimplifiedComplexTest() {
    print("=== 简化复杂测试开始 ===")
    
    // 测试类
    print("\n--- 测试类 ---")
    let animal = Animal(name: "Generic", age: 5)
    print(animal.speak())
    print(animal.getInfo())
    
    let dog = Dog(name: "Buddy", age: 3, breed: "Golden Retriever")
    print(dog.speak())
    print(dog.wagTail())
    print(dog.getInfo())
    
    // 测试结构体
    print("\n--- 测试结构体 ---")
    let point1 = Point(x: 0.0, y: 0.0)
    let point2 = Point(x: 3.0, y: 4.0)
    print("Distance between points: \(point1.distance(to: point2))")
    
    // 测试数组操作
    print("\n--- 测试数组操作 ---")
    let numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    print("Original numbers: \(numbers)")
    
    let doubled = doubleNumbers(numbers)
    print("Doubled: \(doubled)")
    
    let evenNumbers = filterEven(numbers)
    print("Even numbers: \(evenNumbers)")
    
    let sum = calculateSum(numbers)
    print("Sum: \(sum)")
    
    let max = findMax(numbers)
    print("Maximum: \(max)")
    
    // 测试字典
    print("\n--- 测试字典 ---")
    let fruits = ["apple", "banana", "apple", "orange", "banana", "apple"]
    let fruitCounts = countItems(fruits)
    print("Fruit counts: \(fruitCounts)")
    
    // 测试递归函数
    print("\n--- 测试递归函数 ---")
    for i in 1...6 {
        print("Factorial of \(i): \(factorial(i))")
    }
    
    print("\nFibonacci sequence:")
    for i in 0...8 {
        print("F(\(i)) = \(fibonacci(i))")
    }
    
    // 测试控制流
    print("\n--- 测试控制流 ---")
    let testNumbers = [1, 2, 3, 4, 5]
    for num in testNumbers {
        if num % 2 == 0 {
            print("\(num) is even")
        } else {
            print("\(num) is odd")
        }
    }
    
    // 测试switch语句
    print("\n--- 测试Switch语句 ---")
    for i in 1...5 {
        switch i {
        case 1:
            print("One")
        case 2:
            print("Two")
        case 3:
            print("Three")
        case 4:
            print("Four")
        case 5:
            print("Five")
        default:
            print("Other")
        }
    }
    
    // 测试嵌套数组
    print("\n--- 测试嵌套数组 ---")
    let matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
    print("Matrix:")
    for row in matrix {
        print(row)
    }
    
    // 计算矩阵对角线和
    var diagonalSum = 0
    for i in 0..<matrix.count {
        diagonalSum += matrix[i][i]
    }
    print("Diagonal sum: \(diagonalSum)")
    
    // 测试字符串操作
    print("\n--- 测试字符串操作 ---")
    let originalString = "Hello"
    let reversed = reverseString(originalString)
    print("Original: \(originalString)")
    print("Reversed: \(reversed)")
    
    // 测试范围和循环
    print("\n--- 测试范围和循环 ---")
    print("Counting from 1 to 5:")
    for i in 1...5 {
        print("Count: \(i)")
    }
    
    print("\nCounting from 0 to 4:")
    for i in 0..<5 {
        print("Index: \(i)")
    }
    
    print("\n=== 简化复杂测试完成 ===")
}

// 运行测试
runSimplifiedComplexTest()