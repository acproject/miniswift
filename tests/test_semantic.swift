// 测试语义分析器的Swift代码示例

// 变量声明和类型推断
var x = 42
var y: Int = 100
var name = "Hello, Swift!"
var isValid: Bool = true

// 函数定义
func add(a: Int, b: Int) -> Int {
    return a + b
}

func greet(name: String) -> String {
    return "Hello, " + name
}

// 函数调用
var sum = add(a: x, b: y)
var greeting = greet(name: "World")

// 条件语句
if sum > 100 {
    print("Sum is greater than 100")
} else {
    print("Sum is less than or equal to 100")
}

// 循环语句
for i in 1...5 {
    print(i)
}

// 数组
var numbers = [1, 2, 3, 4, 5]
var fruits: [String] = ["apple", "banana", "orange"]

// 字典
var scores = ["Alice": 95, "Bob": 87, "Charlie": 92]

// 结构体定义
struct Point {
    var x: Int
    var y: Int
    
    func distance() -> Double {
        return sqrt(Double(x * x + y * y))
    }
}

// 类定义
class Person {
    var name: String
    var age: Int
    
    init(name: String, age: Int) {
        self.name = name
        self.age = age
    }
    
    func introduce() -> String {
        return "My name is \(name) and I am \(age) years old"
    }
}

// 实例化
var point = Point(x: 3, y: 4)
var person = Person(name: "Alice", age: 25)

// 方法调用
var distance = point.distance()
var introduction = person.introduce()

// 可选类型
var optionalValue: Int? = 42
var nilValue: String? = nil

// 可选绑定
if let value = optionalValue {
    print("Value is \(value)")
}

// 错误处理示例（可能的语义错误）
// var invalidOperation = "string" + 42  // 类型不匹配
// var undeclaredVar = someUndefinedVariable  // 未声明的变量
// func duplicateFunction() {}  // 重复函数定义（如果已存在）