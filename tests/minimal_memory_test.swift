// 最小化的内存管理测试
// 只测试基本的类和属性

class SimpleObject {
    var number: Int
    
    init(number: Int) {
        self.number = number
        print("SimpleObject created with number: \(number)")
    }
    
    deinit {
        print("SimpleObject with number \(number) destroyed")
    }
}

// 测试基本对象创建和属性访问
print("=== 最小化内存管理测试 ===")

var obj = SimpleObject(number: 123)
print("对象创建完成")
print("对象的number属性: \(obj.number)")

// 修改属性
obj.number = 456
print("修改后的number属性: \(obj.number)")

print("测试完成")