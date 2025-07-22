// 最基础的对象测试
// 只测试对象创建和属性访问

class Person {
    var name: String
    var age: Int
    
    init(name: String, age: Int) {
        self.name = name
        self.age = age
        print("Person created: \(name), age \(age)")
    }
    
    deinit {
        print("Person \(name) destroyed")
    }
}

print("=== 基础对象测试 ===")

// 测试1: 创建单个对象
print("\n--- 测试1: 创建单个对象 ---")
var person1 = Person(name: "Alice", age: 25)
print("person1 创建完成")
print("姓名: \(person1.name)")
print("年龄: \(person1.age)")

// 测试2: 修改属性
print("\n--- 测试2: 修改属性 ---")
person1.name = "Alice Smith"
person1.age = 26
print("修改后 - 姓名: \(person1.name)")
print("修改后 - 年龄: \(person1.age)")

// 测试3: 创建多个对象
print("\n--- 测试3: 创建多个对象 ---")
var person2 = Person(name: "Bob", age: 30)
var person3 = Person(name: "Charlie", age: 35)

print("person2 - 姓名: \(person2.name), 年龄: \(person2.age)")
print("person3 - 姓名: \(person3.name), 年龄: \(person3.age)")

// 测试4: 对象引用
print("\n--- 测试4: 对象引用 ---")
var personRef = person1
print("创建引用，原对象姓名: \(person1.name)")
print("引用对象姓名: \(personRef.name)")

personRef.name = "Alice Johnson"
print("通过引用修改后，原对象姓名: \(person1.name)")

print("\n=== 基础对象测试完成 ===")