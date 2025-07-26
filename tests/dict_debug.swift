// 专门测试字典操作

print("=== 测试字典操作 ===")

// 测试字典创建和赋值
var ages: [String: Int] = [:]
print("Empty dict created")

ages["Alice"] = 25
print("Added Alice")

ages["Bob"] = 30
print("Added Bob")

print("Ages: \(ages)")
print("Alice's age: \(ages[\"Alice\"])")

print("=== 字典测试完成 ===")