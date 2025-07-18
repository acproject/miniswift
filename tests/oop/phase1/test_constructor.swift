// Test constructor and destructor interpretation

struct Person {
    var name: String
    var age: Int
    
    init(name: String, age: Int) {
        self.name = name
        self.age = age
        print("Person initialized with name: " + name)
    }
    
    deinit {
        print("Person " + name + " is being deinitialized")
    }
}

class Vehicle {
    var brand: String
    var year: Int
    
    init(brand: String, year: Int) {
        self.brand = brand
        self.year = year
        print("Vehicle initialized: " + brand)
    }
    
    deinit {
        print("Vehicle " + brand + " is being destroyed")
    }
}

// Test struct initialization
var person = Person(name: "Alice", age: 30)
print(person.name)

// Test class initialization  
var car = Vehicle(brand: "Toyota", year: 2023)
print(car.brand)