// 简化的继承测试

class Animal {
    func makeSound() {
        print("Some generic animal sound")
    }
}

class Dog: Animal {
    func makeSound() {
        print("Woof!")
    }
    
    func wagTail() {
        print("Dog is wagging tail")
    }
}

// 测试继承
var myDog = Dog()
print("Dog created")
myDog.makeSound()
print("makeSound called")
myDog.wagTail()
print("wagTail called")