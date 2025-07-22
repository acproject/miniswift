// MiniSwift 内存管理测试
// 测试 ARC、弱引用、无主引用、循环引用检测等功能

// 测试类定义
class Person {
    var name: String
    var apartment: Apartment?
    
    init(name: String) {
        self.name = name
        print("Person \(name) 被创建")
    }
    
    deinit {
        print("Person \(name) 被销毁")
    }
}

class Apartment {
    var unit: String
    weak var tenant: Person?  // 弱引用避免循环引用
    
    init(unit: String) {
        self.unit = unit
        print("Apartment \(unit) 被创建")
    }
    
    deinit {
        print("Apartment \(unit) 被销毁")
    }
}

// 测试无主引用的类
class Customer {
    var name: String
    var card: CreditCard?
    
    init(name: String) {
        self.name = name
        print("Customer \(name) 被创建")
    }
    
    deinit {
        print("Customer \(name) 被销毁")
    }
}

class CreditCard {
    var number: Int
    unowned var customer: Customer  // 无主引用
    
    init(number: Int, customer: Customer) {
        self.number = number
        self.customer = customer
        print("CreditCard \(number) 被创建")
    }
    
    deinit {
        print("CreditCard \(number) 被销毁")
    }
}

// 测试循环引用的类
class Node {
    var value: Int
    var next: Node?
    var previous: Node?
    
    init(value: Int) {
        self.value = value
        print("Node \(value) 被创建")
    }
    
    deinit {
        print("Node \(value) 被销毁")
    }
}

// 主测试函数
func testMemoryManagement() {
    print("=== 开始内存管理测试 ===")
    
    // 测试1: 基本ARC功能
    print("\n--- 测试1: 基本ARC功能 ---")
    var person1: Person? = Person(name: "张三")
    var person2: Person? = person1  // 引用计数增加
    person1 = nil  // 引用计数减少，但对象仍存在
    print("person1 设为 nil，但对象仍存在")
    person2 = nil  // 引用计数为0，对象被销毁
    print("person2 设为 nil，对象被销毁")
    
    // 测试2: 弱引用避免循环引用
    print("\n--- 测试2: 弱引用避免循环引用 ---")
    var john: Person? = Person(name: "约翰")
    var unit4A: Apartment? = Apartment(unit: "4A")
    
    john?.apartment = unit4A
    unit4A?.tenant = john  // 弱引用，不会增加引用计数
    
    john = nil  // Person对象被销毁
    print("john 设为 nil")
    unit4A = nil  // Apartment对象被销毁
    print("unit4A 设为 nil")
    
    // 测试3: 无主引用
    print("\n--- 测试3: 无主引用 ---")
    var customer: Customer? = Customer(name: "李四")
    customer?.card = CreditCard(number: 1234567890, customer: customer!)
    
    customer = nil  // Customer和CreditCard都被销毁
    print("customer 设为 nil，两个对象都被销毁")
    
    // 测试4: 创建循环引用（演示问题）
    print("\n--- 测试4: 循环引用问题演示 ---")
    var node1: Node? = Node(value: 1)
    var node2: Node? = Node(value: 2)
    var node3: Node? = Node(value: 3)
    
    // 创建循环链表
    node1?.next = node2
    node2?.next = node3
    node3?.next = node1  // 创建循环引用
    
    node2?.previous = node1
    node3?.previous = node2
    node1?.previous = node3  // 创建反向循环引用
    
    print("创建了循环引用的链表")
    
    // 设置为nil，但由于循环引用，对象不会被销毁
    node1 = nil
    node2 = nil
    node3 = nil
    print("节点变量设为 nil，但对象可能仍存在（循环引用）")
    
    // 测试5: 数组和字典的内存管理
    print("\n--- 测试5: 集合类型的内存管理 ---")
    var people: [Person] = []
    people.append(Person(name: "王五"))
    people.append(Person(name: "赵六"))
    
    print("数组中有 \(people.count) 个Person对象")
    people.removeAll()
    print("清空数组，对象被销毁")
    
    // 测试6: 可选值的内存管理
    print("\n--- 测试6: 可选值的内存管理 ---")
    var optionalPerson: Person? = Person(name: "孙七")
    print("创建可选Person对象")
    
    if let person = optionalPerson {
        print("可选值解包成功: \(person.name)")
    }
    
    optionalPerson = nil
    print("可选值设为 nil，对象被销毁")
    
    print("\n=== 内存管理测试完成 ===")
}

// 测试弱引用行为
func testWeakReferences() {
    print("\n=== 弱引用行为测试 ===")
    
    var strongPerson: Person? = Person(name: "强引用测试")
    weak var weakPerson: Person? = strongPerson
    
    print("强引用和弱引用都指向同一对象")
    print("弱引用是否为nil: \(weakPerson == nil)")
    
    strongPerson = nil
    print("强引用设为nil后")
    print("弱引用是否为nil: \(weakPerson == nil)")
}

// 测试无主引用行为
func testUnownedReferences() {
    print("\n=== 无主引用行为测试 ===")
    
    var customer: Customer? = Customer(name: "无主引用测试")
    var card: CreditCard? = CreditCard(number: 9876543210, customer: customer!)
    
    print("Customer: \(card?.customer.name ?? "未知")")
    
    // 注意：在实际使用中，需要确保无主引用的对象不会被提前销毁
    customer = nil
    card = nil
    print("Customer和CreditCard都被销毁")
}

// 主程序入口
print("开始MiniSwift内存管理功能测试")
testMemoryManagement()
testWeakReferences()
testUnownedReferences()
print("\n所有测试完成！")