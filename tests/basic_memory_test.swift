// 最基础的内存管理测试
// 只测试对象创建，避免复杂的可选类型操作

class TestClass {
    var value: Int
    
    init(value: Int) {
        self.value = value
        print("TestClass created with value: \(value)")
    }
    
    deinit {
        print("TestClass with value \(value) destroyed")
    }
    
    func getValue() -> Int {
        return self.value
    }
}

// 测试基本对象创建
print("=== 基本对象创建测试 ===")

var obj1 = TestClass(value: 42)
print("对象创建完成，值为: \(obj1.getValue())")

var obj2 = TestClass(value: 100)
print("第二个对象创建完成，值为: \(obj2.getValue())")

// 测试对象方法调用
print("\n=== 对象方法调用测试 ===")
print("obj1的值: \(obj1.getValue())")
print("obj2的值: \(obj2.getValue())")

// 修改对象属性
obj1.value = 999
print("修改obj1的值后: \(obj1.getValue())")

print("\n=== 测试完成 ===")