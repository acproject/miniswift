// 渐进式复杂测试 - 基于minimal_test逐步扩展

print("=== MiniSwift 渐进式复杂测试 ===")

// 第一部分：基础功能（来自minimal_test）
print("\n--- 第一部分：基础功能 ---")
print("Hello from MiniSwift!")

let x = 10
let y = 20
let sum = x + y
print("Sum: \(sum)")

let numbers = [1, 2, 3]
print("Numbers: \(numbers)")

for i in 1...3 {
    print("Count: \(i)")
}

if x > 5 {
    print("x is greater than 5")
}

func add(a: Int, b: Int) -> Int {
    return a + b
}

let result = add(a: 15, b: 25)
print("15 + 25 = \(result)")

// 第二部分：扩展数组操作
print("\n--- 第二部分：扩展数组操作 ---")
let moreNumbers = [4, 5, 6, 7, 8]
print("More numbers: \(moreNumbers)")
print("First element: \(moreNumbers[0])")
print("Array count: \(moreNumbers.count)")

// 第三部分：更多函数
print("\n--- 第三部分：更多函数 ---")
func multiply(a: Int, b: Int) -> Int {
    return a * b
}

func greet(name: String) -> String {
    return "Hello, " + name + "!"
}

let product = multiply(a: 6, b: 7)
print("6 * 7 = \(product)")

let greeting = greet(name: "World")
print(greeting)

// 第四部分：字符串操作
print("\n--- 第四部分：字符串操作 ---")
let firstName = "John"
let lastName = "Doe"
let fullName = firstName + " " + lastName
print("Full name: \(fullName)")

let message = "The result is \(product)"
print(message)

// 第五部分：更多条件语句
print("\n--- 第五部分：更多条件语句 ---")
let a = 15
if a > 10 {
    print("a is greater than 10")
}

let b = 5
if b < 10 {
    print("b is less than 10")
} else {
    print("b is not less than 10")
}

// 第六部分：Switch语句
print("\n--- 第六部分：Switch语句 ---")
let dayNumber = 3
switch dayNumber {
case 1:
    print("Monday")
case 2:
    print("Tuesday")
case 3:
    print("Wednesday")
case 4:
    print("Thursday")
case 5:
    print("Friday")
default:
    print("Weekend")
}

// 第七部分：结构体
print("\n--- 第七部分：结构体 ---")
struct Point {
    var x: Int
    var y: Int
    
    func getX() -> Int {
        return x
    }
    
    func getY() -> Int {
        return y
    }
}

let point = Point(x: 10, y: 20)
print("Point X: \(point.getX())")
print("Point Y: \(point.getY())")
print("Direct access - X: \(point.x), Y: \(point.y)")

// 第八部分：类
print("\n--- 第八部分：类 ---")
class Counter {
    var value: Int
    
    init(startValue: Int) {
        self.value = startValue
    }
    
    func getValue() -> Int {
        return value
    }
    
    func increment() {
        value = value + 1
    }
}

let counter = Counter(startValue: 0)
print("Initial counter: \(counter.getValue())")
counter.increment()
print("After increment: \(counter.getValue())")

// 第九部分：字典
print("\n--- 第九部分：字典 ---")
var ages: [String: Int] = [:]
ages["Alice"] = 25
ages["Bob"] = 30
print("Ages: \(ages)")
print("Alice's age: \(ages[\"Alice\"])")

// 第十部分：递归函数
print("\n--- 第十部分：递归函数 ---")
func factorial(n: Int) -> Int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n: n - 1)
}

let fact4 = factorial(n: 4)
print("4! = \(fact4)")

print("\n=== 渐进式复杂测试完成 ===")