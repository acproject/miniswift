// Comprehensive test for type casting and checking features

// Basic type checking tests
let intVal = 42
let doubleVal = 3.14
let stringVal = "Hello"
let boolVal = true
let arrayVal = [1, 2, 3]
let dictVal = ["key": "value"]

print("=== Type Checking Tests ===")
print("intVal is Int:", intVal is Int)           // true
print("intVal is String:", intVal is String)     // false
print("doubleVal is Double:", doubleVal is Double) // true
print("stringVal is String:", stringVal is String) // true
print("boolVal is Bool:", boolVal is Bool)       // true
print("arrayVal is Array:", arrayVal is Array)   // true
print("dictVal is Dictionary:", dictVal is Dictionary) // true

// Type casting tests
print("\n=== Type Casting Tests ===")

// Safe casting (as) - returns optional
let safeIntToString = intVal as String
print("intVal as String:", safeIntToString)

let safeIntToDouble = intVal as Double
print("intVal as Double:", safeIntToDouble)

// Optional casting (as?) - returns optional
let optionalStringToInt = stringVal as? Int
print("stringVal as? Int:", optionalStringToInt)  // nil

let numberString = "456"
let optionalNumberToInt = numberString as? Int
print("numberString as? Int:", optionalNumberToInt) // Optional(456)

let optionalDoubleToInt = doubleVal as? Int
print("doubleVal as? Int:", optionalDoubleToInt)   // Optional(3)

// Forced casting (as!) - crashes on failure
let forcedIntToString = intVal as! String
print("intVal as! String:", forcedIntToString)     // "42"

let forcedDoubleToInt = doubleVal as! Int
print("doubleVal as! Int:", forcedDoubleToInt)     // 3

// String to number conversions
let validNumber = "789"
let invalidNumber = "abc"

let validConversion = validNumber as? Int
print("validNumber as? Int:", validConversion)     // Optional(789)

let invalidConversion = invalidNumber as? Int
print("invalidNumber as? Int:", invalidConversion) // nil

print("\n=== All tests completed successfully! ===")