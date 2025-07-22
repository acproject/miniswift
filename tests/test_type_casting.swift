// Test file for type casting and checking features

// Test basic type checking with 'is' operator
let intValue = 42
let stringValue = "Hello"
let doubleValue = 3.14
let boolValue = true

print("Type checking tests:")
let test1 = intValue is Int
print(test1)        // Should be true

let test2 = intValue is String
print(test2)  // Should be false

let test3 = stringValue is String
print(test3)  // Should be true

let test4 = doubleValue is Double
print(test4)  // Should be true

let test5 = boolValue is Bool
print(test5)    // Should be true

// Test type casting with 'as?' operator (optional casting)
print("Optional casting tests:")
let optionalInt = stringValue as? Int
print(optionalInt)  // Should be nil

let numberString = "123"
let convertedInt = numberString as? Int
print(convertedInt)  // Should be Optional(123)

// Test type casting with 'as' operator (safe casting)
print("Safe casting tests:")
let stringFromInt = intValue as String
print(stringFromInt)  // Should convert to "42"

let doubleFromInt = intValue as Double
print(doubleFromInt)  // Should convert to 42.0

// Test forced casting with 'as!' operator
print("Forced casting tests:")
let forcedString = intValue as! String
print(forcedString)  // Should convert to "42"

print("All type casting tests completed!")