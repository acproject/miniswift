// Test file for property system enhancements
// Phase 1: Basic property features

// Test 1: Basic stored properties
struct Person {
    var name: String
    let age: Int
    var isAdult: Bool
}

// Test 2: Properties with default values
struct Rectangle {
    var width: Double = 10.0
    var height: Double = 5.0
    let color: String = "red"
}

// Test 3: Computed properties
struct Circle {
    var radius: Double
    
    var area: Double {
        get {
            return 3.14159 * radius * radius
        }
    }
    
    var diameter: Double {
        get {
            return radius * 2
        }
        set {
            radius = newValue / 2
        }
    }
}

// Test 4: Property observers
struct Temperature {
    var celsius: Double {
        willSet {
            print("Temperature will change from \(celsius) to \(newValue)")
        }
        didSet {
            print("Temperature changed from \(oldValue) to \(celsius)")
        }
    }
}

// Test 5: Lazy properties
struct DataManager {
    lazy var expensiveData: String = {
        print("Computing expensive data...")
        return "Computed data"
    }()
    
    var simpleData: String = "Simple"
}

// Test 6: Static properties
struct MathConstants {
    static let pi: Double = 3.14159
    static var counter: Int = 0
}

// Test usage examples
func testProperties() {
    // Test basic properties
    var person = Person(name: "Alice", age: 25, isAdult: true)
    print(person.name)  // Should print: Alice
    person.name = "Bob"
    print(person.name)  // Should print: Bob
    
    // Test default values
    var rect = Rectangle()
    print(rect.width)   // Should print: 10.0
    rect.width = 15.0
    print(rect.width)   // Should print: 15.0
    
    // Test computed properties
    var circle = Circle(radius: 5.0)
    print(circle.area)      // Should print: 78.53975
    print(circle.diameter)  // Should print: 10.0
    circle.diameter = 20.0
    print(circle.radius)    // Should print: 10.0
    
    // Test property observers
    var temp = Temperature(celsius: 20.0)
    temp.celsius = 25.0  // Should trigger willSet and didSet
    
    // Test lazy properties
    var manager = DataManager()
    print(manager.simpleData)    // Should print: Simple
    print(manager.expensiveData) // Should trigger computation and print result
    
    // Test static properties
    print(MathConstants.pi)      // Should print: 3.14159
    MathConstants.counter = 5
    print(MathConstants.counter) // Should print: 5
}

testProperties()