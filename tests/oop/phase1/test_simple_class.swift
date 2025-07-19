class Dog {
    var name: String = "Buddy"
    
    func makeSound() {
        print("Woof!")
    }
}

var myDog = Dog()
print("Dog created")
myDog.makeSound()
print("Method called")