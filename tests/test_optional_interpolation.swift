// 测试可选值的字符串插值
print("开始可选值插值测试")

var ages: [String: Int] = [:]
ages["Alice"] = 25

// 先获取值
let aliceAge = ages["Alice"]
print("获取到的值类型测试")

// 尝试插值可选值
print("Alice's age: \(aliceAge)")
print("测试完成")