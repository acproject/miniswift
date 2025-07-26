// 测试直接字典访问的字符串插值
print("开始直接访问插值测试")

var ages: [String: Int] = [:]
ages["Alice"] = 25
print("字典准备完成")

// 测试直接访问插值
print("Alice's age: \(ages["Alice"])")
print("测试完成")