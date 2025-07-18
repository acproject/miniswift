// MiniSwift Control Flow Test
// Testing if-else statements, while loops, and for loops

print("=== Control Flow Tests ===")

// Test 1: Basic if-else statement
print("\n1. Basic if-else test:")
var x = 10
if x > 5 {
    print("x is greater than 5")
} else {
    print("x is not greater than 5")
}

// Test 2: if-else with false condition
print("\n2. if-else with false condition:")
var y = 3
if y > 5 {
    print("y is greater than 5")
} else {
    print("y is not greater than 5")
}

// Test 3: Nested if statements
print("\n3. Nested if statements:")
var a = 15
if a > 10 {
    print("a is greater than 10")
    if a > 20 {
        print("a is also greater than 20")
    } else {
        print("but a is not greater than 20")
    }
}

// Test 4: While loop
print("\n4. While loop test:")
var count = 0
while count < 3 {
    print("Count: ")
    print(count)
    count = count + 1
}

// Test 5: For loop with initialization, condition, and increment
print("\n5. For loop test:")
for (var i = 0; i < 3; i = i + 1) {
    print("For loop iteration: ")
    print(i)
}

// Test 6: Block scope test
print("\n6. Block scope test:")
var outer = "outer variable"
{
    var inner = "inner variable"
    print(inner)
    print(outer)  // Should access outer variable
}
print(outer)  // Should still work

// Test 7: Variable scope in for loop
print("\n7. For loop variable scope:")
for (var j = 0; j < 2; j = j + 1) {
    var loopVar = "loop variable"
    print(loopVar)
}
// j and loopVar should not be accessible here

print("\n=== Control Flow Tests Complete ===")