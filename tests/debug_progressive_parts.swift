// 逐步测试progressive_test的各个部分

print("=== 测试数组操作 ===")

// 测试数组访问
let moreNumbers = [4, 5, 6, 7, 8]
print("More numbers: \(moreNumbers)")
print("First element: \(moreNumbers[0])")
print("Array count: \(moreNumbers.count)")

print("=== 测试函数 ===")

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

print("=== 测试字符串操作 ===")

let firstName = "John"
let lastName = "Doe"
let fullName = firstName + " " + lastName
print("Full name: \(fullName)")

let message = "The result is \(product)"
print(message)

print("=== 第一部分测试完成 ===")