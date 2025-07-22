// Comprehensive test for multiple arguments in print function
let intVal = 42
let stringVal = "Hello World"
let boolVal = true
let doubleVal = 3.14159

// Test basic multiple arguments
print("Basic test:", "int =", intVal, "string =", stringVal)
print("Bool value:", boolVal, "Double value:", doubleVal)

// Test with complex types
let arr = [1, 2, 3, 4, 5]
let dict = ["name": "Swift", "version": "5.0"]
let tuple = (42, "answer")

print("Complex types:")
print("Array:", arr, "size:", 5)
print("Dictionary:", dict, "keys:", 2)
print("Tuple:", tuple, "elements:", 2)

// Test mixed types in single print
print("Mixed types:", intVal, stringVal, boolVal, doubleVal, arr, dict)
let i = 20
print("场景 \(i): 成功")
print("i: \(i)")