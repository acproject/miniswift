let name = "Alice"
let age = 25
let height = 5.6
let isStudent = true

// 单个插值
print("Name: \(name)")

// 多个插值
print("\(name) is \(age) years old")

// 混合类型插值
print("Student \(name): age=\(age), height=\(height), student=\(isStudent)")

// 以插值开头
print("\(name) studies here")

// 以插值结尾
print("The student is \(name)")

// 连续插值
print("\(name)\(age)\(height)")