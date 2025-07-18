// Test enum declarations and usage

// Simple enum without associated values
enum Color {
    case red
    case green
    case blue
}

// Enum with associated values
enum Shape {
    case circle(Double)
    case rectangle(Double, Double)
    case triangle(Double, Double, Double)
}

// Test enum usage
print("Testing enum declarations...")

// Create enum values
let myColor = Color.red
let myShape = Shape.circle(5.0)
let myRect = Shape.rectangle(10.0, 20.0)

// Print enum values
print("Color: ")
print(myColor)
print("Shape: ")
print(myShape)
print("Rectangle: ")
print(myRect)

print("Enum test completed!")