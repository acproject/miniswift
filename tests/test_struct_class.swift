// Test struct and class declarations

// Define a struct
struct Person {
    var name: String
    var age: Int
}

// Define a class
class Car {
    var brand: String
    var year: Int
}

// Create struct instance
var person = Person(name: "Alice", age: 30)
print(person)
print(person.name)
print(person.age)

// Create class instance
var car = Car(brand: "Toyota", year: 2020)
print(car)
print(car.brand)
print(car.year)

// Test member access
person.name = "Bob"
person.age = 25
print(person)

car.brand = "Honda"
car.year = 2021
print(car)