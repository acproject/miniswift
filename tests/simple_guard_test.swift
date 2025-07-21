// 简单的 Guard 语句测试
print("=== Guard 语句测试 ===")

// 测试基本的 guard 语句
func checkValue(value: Int) -> String {
     guard value > 0 else {
        print("值必须大于0")
        return "失败"
    }
    
    return "成功"
}

// 测试 guard 语句
let result1 = checkValue(value: 5)
print(result1)

let result2 = checkValue(value: -1)
print(result2)

print("测试完成")