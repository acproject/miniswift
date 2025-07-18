// Simple property test

// Test 1: Basic stored properties
struct Person {
    var name: String
    let age: Int
}

// Test 2: Properties with default values
struct Rectangle {
    var width: Double = 10.0
    var height: Double = 5.0
}

// Test 3: Simple computed property
struct Circle {
    var radius: Double
    
    var diameter: Double {
        get {
            return radius * 2
        }
        set {
            radius = newValue / 2
        }
    }
}

// Test usage
var person = Person(name: "Alice", age: 25)
print(person.name)
print(person.age)

var rect = Rectangle()
print(rect.width)
print(rect.height)

var circle = Circle(radius: 5.0)
print(circle.diameter)
circle.diameter = 20.0
print(circle.radius)