// 测试简单的类方法调用

class SimpleClass {
    var name: String = "Test"
    
    func getName() -> String {
        return name
    }
}

let obj = SimpleClass()
print("Name: " + obj.getName())