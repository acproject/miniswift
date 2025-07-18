// Comprehensive Control Flow Test
// Testing control flow with arrays, dictionaries, and other features

print("=== Comprehensive Control Flow Test ===")

// Test 1: Control flow with arrays
print("\n1. Control flow with arrays:")
var numbers = [1, 2, 3, 4, 5]
print("Array: ")
print(numbers)

for (var i = 0; i < 5; i = i + 1) {
    var current = numbers[i]
    if current > 3 {
        print("Large number: ")
        print(current)
    } else {
        print("Small number: ")
        print(current)
    }
}

// Test 2: Control flow with dictionaries
print("\n2. Control flow with dictionaries:")
var scores = ["Alice": 95, "Bob": 87, "Charlie": 92]
print("Scores dictionary: ")
print(scores)

var aliceScore = scores["Alice"]
if aliceScore > 90 {
    print("Alice has an excellent score!")
}

// Test 3: Nested arrays with control flow
print("\n3. Nested arrays with control flow:")
var matrix = [[1, 2], [3, 4], [5, 6]]
print("Matrix: ")
print(matrix)

for (var row = 0; row < 3; row = row + 1) {
    var currentRow = matrix[row]
    print("Processing row: ")
    print(currentRow)
    
    for (var col = 0; col < 2; col = col + 1) {
        var element = currentRow[col]
        if element > 3 {
            print("  Large element: ")
            print(element)
        }
    }
}

// Test 4: String processing with control flow
print("\n4. String processing with control flow:")
var messages = ["Hello\nWorld", "Swift\tProgramming", "Control\"Flow\""]
print("Messages array: ")
print(messages)

for (var i = 0; i < 3; i = i + 1) {
    var msg = messages[i]
    print("Message ")
    print(i)
    print(": ")
    print(msg)
}

// Test 5: Complex conditional logic
print("\n5. Complex conditional logic:")
var data = [10, 25, 30, 45, 50]
var threshold = 30

for (var idx = 0; idx < 5; idx = idx + 1) {
    var value = data[idx]
    
    if value < threshold {
        print("Value ")
        print(value)
        print(" is below threshold")
    } else if value == threshold {
        print("Value ")
        print(value)
        print(" equals threshold")
    } else {
        print("Value ")
        print(value)
        print(" is above threshold")
    }
}

// Test 6: Variable scope with collections
print("\n6. Variable scope with collections:")
var globalArray = [100, 200, 300]
{
    var localArray = [1, 2, 3]
    print("Local array: ")
    print(localArray)
    print("Global array: ")
    print(globalArray)
    
    for (var i = 0; i < 3; i = i + 1) {
        var sum = globalArray[i] + localArray[i]
        print("Sum at index ")
        print(i)
        print(": ")
        print(sum)
    }
}

// Test 7: While loop with array processing
print("\n7. While loop with array processing:")
var countdown = [5, 4, 3, 2, 1]
var index = 0

while index < 5 {
    var current = countdown[index]
    print("Countdown: ")
    print(current)
    
    if current == 1 {
        print("Blast off!")
    }
    
    index = index + 1
}

print("\n=== Comprehensive Control Flow Test Complete ===")