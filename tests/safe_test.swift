// 安全测试 - 避免可能有问题的语法

print("=== MiniSwift 安全测试 ===")

// 1. 基础数据类型
print("\n--- 基础数据类型 ---")
let intValue = 42
let doubleValue = 3.14
let stringValue = "Hello"

print("Int: \(intValue)")
print("Double: \(doubleValue)")
print("String: \(stringValue)")

// 2. 数组基础操作
print("\n--- 数组操作 ---")
let numbers = [1, 2, 3, 4, 5]
print("Numbers: \(numbers)")
print("First: \(numbers[0])")
print("Count: \(numbers.count)")

// 3. 简单循环
print("\n--- 循环测试 ---")
for i in 1...3 {
    print("Loop iteration: \(i)")
}

for num in numbers {
    print("Number: \(num)")
}

// 4. 简单条件语句（分开测试）
print("\n--- 条件语句 ---")
let x = 10
if x > 5 {
    print("x is greater than 5")
}

let y = 3
if y < 5 {
    print("y is less than 5")
}

// 5. Switch语句
print("\n--- Switch语句 ---")
let testValue = 2
switch testValue {
case 1:
    print("One")
case 2:
    print("Two")
case 3:
    print("Three")
default:
    print("Other")
}

// 6. 函数定义和调用
print("\n--- 函数测试 ---")
func add(a: Int, b: Int) -> Int {
    return a + b
}

func multiply(a: Int, b: Int) -> Int {
    return a * b
}

let sum = add(a: 10, b: 20)
let product = multiply(a: 6, b: 7)
print("10 + 20 = \(sum)")
print("6 * 7 = \(product)")

// 7. 字符串操作
print("\n--- 字符串操作 ---")
let firstName = "John"
let lastName = "Doe"
let fullName = firstName + " " + lastName
print("Full name: \(fullName)")

// 8. 结构体
print("\n--- 结构体测试 ---")
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

// 9. 类
print("\n--- 类测试 ---")
class SimpleClass {
    var value: Int
    
    init(initialValue: Int) {
        self.value = initialValue
    }
    
    func getValue() -> Int {
        return value
    }
    
    func setValue(newValue: Int) {
        value = newValue
    }
}

let obj = SimpleClass(initialValue: 100)
print("Initial value: \(obj.getValue())")
obj.setValue(newValue: 200)
print("New value: \(obj.getValue())")

// 10. 字典基础操作
print("\n--- 字典测试 ---")
var dict: [String: Int] = [:]
dict["key1"] = 10
dict["key2"] = 20

print("Dictionary: \(dict)")
print("key1 value: \(dict[\"key1\"])")

// 11. 嵌套数组
print("\n--- 嵌套数组 ---")
let matrix = [[1, 2], [3, 4]]
print("Matrix: \(matrix)")
print("Element [0][0]: \(matrix[0][0])")
print("Element [1][1]: \(matrix[1][1])")

// 12. 递归函数
print("\n--- 递归函数 ---")
func factorial(n: Int) -> Int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n: n - 1)
}

let fact4 = factorial(n: 4)
let fact5 = factorial(n: 5)
print("4! = \(fact4)")
print("5! = \(fact5)")

print("\n=== 安全测试完成 ===")