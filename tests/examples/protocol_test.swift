// Protocol System Test
// This file tests the basic protocol declaration and conformance functionality

// Define a simple protocol
protocol Drawable {
    var color: String { get set }
    func draw()
    func area() -> Double
}

// Define a protocol with inheritance
protocol Shape: Drawable {
    var width: Double { get }
    var height: Double { get }
}

// Define a struct that conforms to a protocol
struct Rectangle: Shape {
    var width: Double
    var height: Double
    var color: String
    
    func draw() {
        print("Drawing a " + color + " rectangle")
    }
    
    func area() -> Double {
        return width * height
    }
}

// Define a class that conforms to a protocol
class Circle: Drawable {
    var radius: Double
    var color: String
    
    init(radius: Double, color: String) {
        self.radius = radius
        self.color = color
    }
    
    func draw() {
        print("Drawing a " + color + " circle")
    }
    
    func area() -> Double {
        return 3.14159 * radius * radius
    }
}

// Test protocol functionality
var rect = Rectangle(width: 10.0, height: 5.0, color: "red")
rect.draw()
print("Rectangle area: " + String(rect.area()))

var circle = Circle(radius: 3.0, color: "blue")
circle.draw()
print("Circle area: " + String(circle.area()))