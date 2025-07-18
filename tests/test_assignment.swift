// Test various assignment scenarios

// Define a struct
struct Point {
    var x: Int
    var y: Int
}

// Define a class
class Rectangle {
    var width: Int
    var height: Int
    var origin: Point
}

// Test simple variable assignment
var number = 10
print(number)
number = 20
print(number)

// Test struct member assignment
var point = Point(x: 1, y: 2)
print(point)
point.x = 5
point.y = 10
print(point)

// Test class member assignment
var rect = Rectangle(width: 100, height: 50, origin: Point(x: 0, y: 0))
print(rect)
rect.width = 200
rect.height = 100
print(rect)

// Test nested member assignment
rect.origin.x = 10
rect.origin.y = 20
print(rect)
print(rect.origin)