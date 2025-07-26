class Animal {
    func speak() {
        print("Animal makes a sound")
    }
}

class Dog : Animal {
    override func speak() {
        print("Dog barks")
    }
}

let dog = Dog()
dog.speak()