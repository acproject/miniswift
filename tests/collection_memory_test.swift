// 集合类型内存管理测试
// 测试数组中对象的内存管理

class Item {
    var name: String
    var value: Int
    
    init(name: String, value: Int) {
        self.name = name
        self.value = value
        print("Item '\(name)' created with value \(value)")
    }
    
    deinit {
        print("Item '\(name)' destroyed")
    }
}

print("=== 集合类型内存管理测试 ===")

// 测试1: 数组中的对象
print("\n--- 测试1: 数组中的对象 ---")
var items: [Item] = []
print("创建空数组")

// 添加对象到数组
items.append(Item(name: "item1", value: 100))
items.append(Item(name: "item2", value: 200))
items.append(Item(name: "item3", value: 300))

print("数组中有 \(items.count) 个对象")

// 访问数组中的对象
if items.count > 0 {
    print("第一个对象: name=\(items[0].name), value=\(items[0].value)")
}

// 修改数组中对象的属性
if items.count > 1 {
    items[1].value = 999
    print("修改第二个对象的value为: \(items[1].value)")
}

// 测试2: 从数组中移除对象
print("\n--- 测试2: 从数组中移除对象 ---")
if items.count > 0 {
    items.remove(at: 0)
    print("移除第一个对象，剩余 \(items.count) 个对象")
}

// 测试3: 清空数组
print("\n--- 测试3: 清空数组 ---")
items.removeAll()
print("清空数组，剩余 \(items.count) 个对象")

print("\n=== 集合类型内存管理测试完成 ===")