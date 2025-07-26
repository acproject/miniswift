// 复杂综合测试 - 测试MiniSwift的各种高级功能

// 1. 类定义和继承
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

class Dog : Animal {
    var breed: String
    
    init(name: String, age: Int, breed: String) {
        self.breed = breed
        super.init(name: name, age: age)
    }
    
    override func speak() -> String {
        return "\(name) barks: Woof!"
    }
    
    func wagTail() -> String {
        return "\(name) is wagging tail happily!"
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
    
    mutating func move(by deltaX: Double, deltaY: Double) {
        x += deltaX
        y += deltaY
    }
}

// 3. 枚举
enum Color {
    case red
    case green
    case blue
    case custom(r: Int, g: Int, b: Int)
    
    func description() -> String {
        switch self {
        case .red:
            return "Red"
        case .green:
            return "Green"
        case .blue:
            return "Blue"
        case .custom(let r, let g, let b):
            return "Custom RGB(\(r), \(g), \(b))"
        }
    }
}

// 4. 高阶函数和闭包
func processNumbers(_ numbers: [Int], with operation: (Int) -> Int) -> [Int] {
    var result: [Int] = []
    for num in numbers {
        result.append(operation(num))
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

// 5. 字典操作函数
func mergeDictionaries(_ dict1: [String: Int], _ dict2: [String: Int]) -> [String: Int] {
    var result = dict1
    for (key, value) in dict2 {
        if let existingValue = result[key] {
            result[key] = existingValue + value
        } else {
            result[key] = value
        }
    }
    return result
}

// 6. 递归函数
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

// 7. 主测试函数
func runComplexTest() {
    print("=== 复杂综合测试开始 ===")
    
    // 测试类和继承
    print("\n--- 测试类和继承 ---")
    let animal = Animal(name: "Generic", age: 5)
    print(animal.speak())
    print(animal.getInfo())
    
    let dog = Dog(name: "Buddy", age: 3, breed: "Golden Retriever")
    print(dog.speak())
    print(dog.wagTail())
    print(dog.getInfo())
    
    // 测试结构体
    print("\n--- 测试结构体 ---")
    var point1 = Point(x: 0.0, y: 0.0)
    let point2 = Point(x: 3.0, y: 4.0)
    print("Distance: \(point1.distance(to: point2))")
    
    point1.move(by: 1.0, deltaY: 1.0)
    print("Point1 after move: (\(point1.x), \(point1.y))")
    
    // 测试枚举
    print("\n--- 测试枚举 ---")
    let red = Color.red
    let custom = Color.custom(r: 255, g: 128, b: 0)
    print("Red color: \(red.description())")
    print("Custom color: \(custom.description())")
    
    // 测试数组和高阶函数
    print("\n--- 测试数组和高阶函数 ---")
    let numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    print("Original numbers: \(numbers)")
    
    let doubled = processNumbers(numbers) { $0 * 2 }
    print("Doubled: \(doubled)")
    
    let squared = processNumbers(numbers) { $0 * $0 }
    print("Squared: \(squared)")
    
    let evenNumbers = filterEven(numbers)
    print("Even numbers: \(evenNumbers)")
    
    // 测试字典
    print("\n--- 测试字典 ---")
    let dict1: [String: Int] = ["apple": 5, "banana": 3, "orange": 2]
    let dict2: [String: Int] = ["apple": 2, "grape": 4, "orange": 1]
    
    print("Dictionary 1: \(dict1)")
    print("Dictionary 2: \(dict2)")
    
    let merged = mergeDictionaries(dict1, dict2)
    print("Merged dictionary: \(merged)")
    
    // 测试递归函数
    print("\n--- 测试递归函数 ---")
    for i in 1...6 {
        print("Factorial of \(i): \(factorial(i))")
    }
    
    print("\nFibonacci sequence:")
    for i in 0...10 {
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
        case 4, 5:
            print("Four or Five")
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
    
    print("\n=== 复杂综合测试完成 ===")
}

// 运行测试
runComplexTest()