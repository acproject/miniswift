// 基础安全测试 - 不包含类和结构体

print("=== MiniSwift 基础安全测试 ===")

// 基础功能
print("\n--- 基础功能 ---")
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

// 数组操作
print("\n--- 数组操作 ---")
let moreNumbers = [4, 5, 6, 7, 8]
print("More numbers: \(moreNumbers)")
print("First element: \(moreNumbers[0])")

// 函数
print("\n--- 函数 ---")
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

// 字符串操作
print("\n--- 字符串操作 ---")
let firstName = "John"
let lastName = "Doe"
let fullName = firstName + " " + lastName
print("Full name: \(fullName)")

let message = "The result is \(product)"
print(message)

// 条件语句
print("\n--- 条件语句 ---")
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

// Switch语句
print("\n--- Switch语句 ---")
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

// 递归函数
print("\n--- 递归函数 ---")
func factorial(n: Int) -> Int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n: n - 1)
}

let fact4 = factorial(n: 4)
print("4! = \(fact4)")

print("\n=== 基础安全测试完成 ===")