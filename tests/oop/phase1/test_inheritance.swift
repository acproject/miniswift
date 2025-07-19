// 测试继承功能的基本语法

// 定义基类
class Animal {
    var name: String
    
    func makeSound() {
        print("Some generic animal sound")
    }
    
    func describe() {
        print("This is an animal named " + name)
    }
}

// 定义子类
class Dog: Animal {
    var breed: String
    
    func makeSound() {
        print("Woof!")
    }
    
    func wagTail() {
        print(name + " is wagging tail")
    }
}

// 定义另一个子类
class Cat: Animal {
    var isIndoor: Bool
    
    func makeSound() {
        print("Meow!")
    }
    
    func purr() {
        print(name + " is purring")
    }
}

// 测试继承
var myDog = Dog()
myDog.name = "Buddy"
myDog.breed = "Golden Retriever"

var myCat = Cat()
myCat.name = "Whiskers"
myCat.isIndoor = true

// 测试方法调用
myDog.makeSound()  // 应该输出 "Woof!"
myDog.describe()   // 继承自Animal的方法
myDog.wagTail()    // Dog特有的方法

myCat.makeSound()  // 应该输出 "Meow!"
myCat.describe()   // 继承自Animal的方法
myCat.purr()       // Cat特有的方法