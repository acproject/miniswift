// 测试super关键字功能

class Animal {
    var name: String = "Unknown"
    
    func speak() {
        print("Animal makes a sound")
    }
    
    func getName() -> String {
        return name
    }
}

class Dog : Animal {
    override func speak() {
        super.speak()  // 调用父类方法
        print("Dog barks")
    }
    
    override func getName() -> String {
        let parentName = super.getName()  // 调用父类方法获取名称
        return "Dog named " + parentName
    }
}

let dog = Dog()
dog.name = "Buddy"
print("Name: " + dog.getName())
dog.speak()