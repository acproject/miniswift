// 测试构造器和析构器功能

// 定义一个简单的结构体
struct Person {
    var name: String
    var age: Int
    
    // 构造器
    init(name: String, age: Int) {
        self.name = name
        self.age = age
        print("Person created: " + name)
    }
    
    // 析构器
    deinit {
        print("Person destroyed: " + self.name)
    }
}

// 定义一个类
class Student {
    var name: String
    var grade: Int
    
    // 构造器
    init(name: String, grade: Int) {
        self.name = name
        self.grade = grade
        print("Student created: " + name)
    }
    
    // 析构器
    deinit {
        print("Student destroyed: " + self.name)
    }
}

// 测试代码
print("=== 测试构造器和析构器 ===")

// 创建Person实例
var person = Person(name: "Alice", age: 25)
print("Person name: " + person.name)
print("Person age: " + String(person.age))

// 创建Student实例
var student = Student(name: "Bob", grade: 90)
print("Student name: " + student.name)
print("Student grade: " + String(student.grade))

print("=== 测试完成 ===")
