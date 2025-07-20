let name = "Alice"
let age = 25
let height = 5.6
let isStudent = true

print("=== MiniSwift 字符串插值功能测试 ===")

// 1. 基本插值
print("1. 基本插值:")
print("   Name: \(name)")
print("   \(name) is \(age) years old")

// 2. 混合类型插值
print("\n2. 混合类型插值:")
print("   Student \(name): age=\(age), height=\(height), student=\(isStudent)")

// 3. 比较表达式插值
print("\n3. 比较表达式插值:")
print("   Is minor: \(age < 18)")
print("   Is adult: \(age >= 18)")
print("   Student status: \(name): age=\(age), is_minor=\(age < 18)")

// 4. 算术表达式插值
print("\n4. 算术表达式插值:")
print("   \(name) will be \(age + 1) next year")
print("   Height in cm: \(height * 30.48)")
print("   Age doubled: \(age * 2)")

// 5. 连续插值
print("\n5. 连续插值:")
print("   \(name)\(age)")
print("   \(age)\(height)")

// 6. 以插值开头和结尾
print("\n6. 以插值开头和结尾:")
print("   \(name) studies here")
print("   The student is \(name)")

print("\n=== 测试完成 ===")