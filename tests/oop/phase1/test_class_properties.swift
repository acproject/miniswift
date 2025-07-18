// Test file for class property system
// Phase 1: Class properties and inheritance basics

// Test 1: Basic class with stored properties
class Vehicle {
    var brand: String
    var year: Int
    let type: String = "Vehicle"
    
    init(brand: String, year: Int) {
        self.brand = brand
        self.year = year
    }
}

// Test 2: Class with computed properties
class Car : Vehicle {
    var mileage: Double = 0.0
    
    var age: Int {
        get {
            return 2024 - year
        }
    }
    
    var description: String {
        get {
            return "\(brand) \(type) from \(year)"
        }
        set {
            // Parse the description to update brand and year
            // This is a simplified example
            print("Setting description to: \(newValue)")
        }
    }
}

// Test 3: Class with property observers
class BankAccount {
    var balance: Double {
        willSet {
            print("Balance will change from $\(balance) to $\(newValue)")
        }
        didSet {
            if balance < 0 {
                print("Warning: Account is overdrawn!")
            }
        }
    }
    
    init(initialBalance: Double) {
        self.balance = initialBalance
    }
}

// Test 4: Class with lazy properties
class ImageProcessor {
    var imageName: String
    
    lazy var processedImage: String = {
        print("Processing image: \(imageName)")
        return "Processed_\(imageName)"
    }()
    
    init(imageName: String) {
        self.imageName = imageName
    }
}

// Test 5: Class with static properties
class GameSettings {
    static var maxPlayers: Int = 4
    static let gameName: String = "MiniSwift Game"
    
    var playerName: String
    
    init(playerName: String) {
        self.playerName = playerName
    }
}

// Test usage examples
func testClassProperties() {
    // Test basic class properties
    var vehicle = Vehicle(brand: "Toyota", year: 2020)
    print(vehicle.brand)  // Should print: Toyota
    vehicle.brand = "Honda"
    print(vehicle.brand)  // Should print: Honda
    
    // Test inheritance and computed properties
    var car = Car(brand: "BMW", year: 2018)
    print(car.age)         // Should print: 6 (2024 - 2018)
    print(car.description) // Should print: BMW Vehicle from 2018
    car.description = "Luxury BMW from 2018"
    
    // Test property observers
    var account = BankAccount(initialBalance: 1000.0)
    account.balance = 500.0   // Should trigger willSet and didSet
    account.balance = -100.0  // Should trigger warning
    
    // Test lazy properties
    var processor = ImageProcessor(imageName: "photo.jpg")
    print(processor.imageName)      // Should print: photo.jpg
    print(processor.processedImage) // Should trigger processing
    
    // Test static properties
    print(GameSettings.gameName)    // Should print: MiniSwift Game
    GameSettings.maxPlayers = 8
    print(GameSettings.maxPlayers)  // Should print: 8
    
    var game1 = GameSettings(playerName: "Alice")
    var game2 = GameSettings(playerName: "Bob")
    print(game1.playerName)         // Should print: Alice
    print(game2.playerName)         // Should print: Bob
}

testClassProperties()