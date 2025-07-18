// Test file for method system implementation
// Testing current method support and identifying gaps

print("=== Method System Test ===")

// Test 1: Basic struct with simple method
struct Counter {
    var count: Int = 0
    
    func increment() {
        count = count + 1
    }
    
    func getValue() -> Int {
        return count
    }
}

// Test 2: Method with parameters
struct Calculator {
    func add(a: Int, b: Int) -> Int {
        return a + b
    }
    
    func multiply(x: Int, y: Int) -> Int {
        return x * y
    }
}

// Test 3: Method accessing properties
struct Rectangle {
    var width: Double = 0.0
    var height: Double = 0.0
    
    func area() -> Double {
        return width * height
    }
    
    func perimeter() -> Double {
        return 2 * (width + height)
    }
}

// Test 4: Method with self keyword (if supported)
struct Person {
    var name: String = "Unknown"
    
    func greet() {
        print("Hello, I am " + name)
    }
    
    func setName(newName: String) {
        name = newName
    }
}

print("\n1. Testing basic method calls:")
var counter = Counter()
print("Initial count: ")
print(counter.getValue())
counter.increment()
print("After increment: ")
print(counter.getValue())

print("\n2. Testing methods with parameters:")
var calc = Calculator()
var sum = calc.add(a: 5, b: 3)
print("5 + 3 = ")
print(sum)
var product = calc.multiply(x: 4, y: 6)
print("4 * 6 = ")
print(product)

print("\n3. Testing methods accessing properties:")
var rect = Rectangle(width: 5.0, height: 3.0)
print("Rectangle area: ")
print(rect.area())
print("Rectangle perimeter: ")
print(rect.perimeter())

print("\n4. Testing methods with property modification:")
var person = Person()
person.greet()
person.setName(newName: "Alice")
person.greet()

print("\n=== Method System Test Complete ===")