let name = "Alice"
let age = 25
let height = 5.6
let isStudent = true

// 基本插值测试
print("Name: \(name)")
print("\(name) is \(age) years old")

// 复杂混合插值
print("Student \(name): age=\(age), height=\(height), student=\(isStudent)")

// 带比较表达式的插值
print("Student \(name): age=\(age), height=\(height), student=\(age < 18)")

// 以插值开头和结尾
print("\(name) studies here")
print("The student is \(name)")

// 连续插值
print("\(name)\(age)")

// 带算术表达式的插值
print("\(name) will be \(age + 1) next year")
print("Height in cm: \(height * 30.48)")

// 复杂表达式插值
print("Is adult: \(age >= 18)")
// print("Status: \(isStudent && age < 30)")

// 嵌套表达式
// print("Complex: \(age + (height * 2))")