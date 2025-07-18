// Advanced enum test with different types of associated values

// Enum with mixed associated value types
enum Result {
    case success(String)
    case failure(Int)
    case pending
}

// Enum with multiple associated values
enum Person {
    case student(String, Int)  // name, age
    case teacher(String, String)  // name, subject
    case visitor
}

print("Advanced enum testing...")

// Test different enum cases
let result1 = Result.success("Operation completed")
let result2 = Result.failure(404)
let result3 = Result.pending

let person1 = Person.student("Alice", 20)
let person2 = Person.teacher("Bob", "Math")
let person3 = Person.visitor

print("Results:")
print(result1)
print(result2)
print(result3)

print("People:")
print(person1)
print(person2)
print(person3)

print("Advanced enum test completed!")