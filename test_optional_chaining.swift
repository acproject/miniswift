// Test optional chaining implementation

// Test 1: Basic optional property access
struct Person {
    var name: String
    var age: Int
}

var person: Person? = Person(name: "Alice", age: 30)
var nilPerson: Person? = nil

// Optional chaining for property access
print(person?.name)  // Should print Optional("Alice")
print(nilPerson?.name)  // Should print nil

// Test 2: Optional chaining with methods (if supported)
// This would require method implementation in structs

// Test 3: Optional chaining with subscripts
var numbers: [Int]? = [1, 2, 3, 4, 5]
var nilArray: [Int]? = nil

print(numbers?[0])  // Should print Optional(1)
print(nilArray?[0])  // Should print nil

// Test 4: Multi-level optional chaining
struct Address {
    var street: String
    var city: String
}

struct PersonWithAddress {
    var name: String
    var address: Address?
}

var personWithAddr: PersonWithAddress? = PersonWithAddress(
    name: "Bob", 
    address: Address(street: "123 Main St", city: "Anytown")
)

print(personWithAddr?.address?.street)  // Should print Optional("123 Main St")
print(personWithAddr?.address?.city)    // Should print Optional("Anytown")

var personNoAddr: PersonWithAddress? = PersonWithAddress(name: "Charlie", address: nil)
print(personNoAddr?.address?.street)    // Should print nil