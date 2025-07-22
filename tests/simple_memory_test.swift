// 简单的内存管理测试
// 测试基本的对象创建和销毁

class SimpleClass {
    var value: Int
    
    init(value: Int) {
        self.value = value
        print("SimpleClass created with value: \(value)")
    }
    
    deinit {
        print("SimpleClass with value \(value) destroyed")
    }
}

// 测试基本对象生命周期
print("=== 基本对象生命周期测试 ===")

// 创建对象
var obj1: SimpleClass? = SimpleClass(value: 42)
print("对象创建完成")

// 销毁对象
obj1 = nil
print("对象销毁完成")

// 测试多个引用
print("\n=== 多引用测试 ===")
var obj2: SimpleClass? = SimpleClass(value: 100)
var obj3: SimpleClass? = obj2  // 增加引用计数

print("创建了两个引用")

obj2 = nil
print("第一个引用设为nil，对象仍存在")

obj3 = nil
print("第二个引用设为nil，对象被销毁")

// 测试数组中的对象
print("\n=== 数组中对象测试 ===")
var objects: [SimpleClass] = []
objects.append(SimpleClass(value: 1))
objects.append(SimpleClass(value: 2))
objects.append(SimpleClass(value: 3))

print("数组中有 \(objects.count) 个对象")

objects.removeAll()
print("清空数组，所有对象被销毁")

print("\n=== 测试完成 ===")