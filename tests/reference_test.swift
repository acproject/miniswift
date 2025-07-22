// 引用计数测试
// 测试对象引用和生命周期

class RefTestClass {
    var id: Int
    
    init(id: Int) {
        self.id = id
        print("RefTestClass \(id) created")
    }
    
    deinit {
        print("RefTestClass \(id) destroyed")
    }
}

print("=== 引用计数测试 ===")

// 测试1: 基本引用
print("\n--- 测试1: 基本对象创建 ---")
var ref1 = RefTestClass(id: 1)
print("ref1 创建完成，id: \(ref1.id)")

// 测试2: 多个引用指向同一对象
print("\n--- 测试2: 多重引用 ---")
var ref2 = RefTestClass(id: 2)
var ref3 = ref2  // 应该增加引用计数
print("ref2 和 ref3 都指向同一对象")
print("ref2.id: \(ref2.id)")
print("ref3.id: \(ref3.id)")

// 修改一个引用的属性，另一个也应该看到变化
ref2.id = 999
print("修改ref2.id后，ref3.id: \(ref3.id)")

// 测试3: 作用域测试
print("\n--- 测试3: 作用域测试 ---")
if true {
    var scopedRef = RefTestClass(id: 3)
    print("作用域内创建对象，id: \(scopedRef.id)")
    // 离开作用域时对象应该被销毁
}
print("离开作用域")

print("\n=== 引用计数测试完成 ===")