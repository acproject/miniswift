// 最小测试 - 验证基本功能

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

print("Test completed successfully!")