// Test multiple arguments in print function
let intVal = 42
let stringVal = "Hello"
let boolVal = true
let doubleVal = 3.14

// Test print with multiple arguments
print("intVal is Int:", intVal is Int)
print("Value:", intVal, "Type:", "Int")
print("String:", stringVal, "Bool:", boolVal)
print("Multiple types:", intVal, stringVal, boolVal, doubleVal)

// Test type casting with multiple arguments
print("Safe cast:", intVal as? String)
print("Type check:", stringVal is String)
print("Forced cast result:", doubleVal as! Double)

// Test with arrays and other complex types
let arr = [1, 2, 3]
let dict = ["key": "value"]
print("Array:", arr, "Dictionary:", dict)