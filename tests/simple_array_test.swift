// 简单数组测试
// 测试基本的数组操作和对象存储

class SimpleItem {
    var id: Int
    
    init(id: Int) {
        self.id = id
        print("SimpleItem \(id) created")
    }
    
    deinit {
        print("SimpleItem \(id) destroyed")
    }
}

print("=== 简单数组测试 ===")

// 测试1: 创建对象
print("\n--- 测试1: 创建对象 ---")
var item1 = SimpleItem(id: 1)
var item2 = SimpleItem(id: 2)
var item3 = SimpleItem(id: 3)

print("创建了3个对象")
print("item1.id: \(item1.id)")
print("item2.id: \(item2.id)")
print("item3.id: \(item3.id)")

// 测试2: 数组字面量
print("\n--- 测试2: 数组字面量 ---")
var numbers = [1, 2, 3, 4, 5]
print("创建数字数组，长度: \(numbers.count)")

if numbers.count > 0 {
    print("第一个数字: \(numbers[0])")
}

if numbers.count > 2 {
    print("第三个数字: \(numbers[2])")
}

// 测试3: 字符串数组
print("\n--- 测试3: 字符串数组 ---")
var names = ["Alice", "Bob", "Charlie"]
print("创建字符串数组，长度: \(names.count)")

if names.count > 1 {
    print("第二个名字: \(names[1])")
}

print("\n=== 简单数组测试完成 ===")