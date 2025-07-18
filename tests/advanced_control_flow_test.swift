// Advanced Control Flow Test
// Testing more complex control flow scenarios

print("=== Advanced Control Flow Tests ===")

// Test 1: else if chain
print("\n1. else if chain test:")
var score = 85
if score >= 90 {
    print("Grade: A")
} else if score >= 80 {
    print("Grade: B")
} else if score >= 70 {
    print("Grade: C")
} else {
    print("Grade: F")
}

// Test 2: Nested loops
print("\n2. Nested loops test:")
for (var i = 1; i <= 2; i = i + 1) {
    print("Outer loop: ")
    print(i)
    for (var j = 1; j <= 2; j = j + 1) {
        print("  Inner loop: ")
        print(j)
    }
}

// Test 3: While loop with simple condition
print("\n3. While loop with simple condition:")
var x = 1
while x < 4 {
    print("x: ")
    print(x)
    x = x + 1
}

// Test 4: For loop with different increment
print("\n4. For loop with step 2:")
for (var k = 0; k < 10; k = k + 2) {
    print("Even number: ")
    print(k)
}

// Test 5: Conditional with boolean variables
print("\n5. Boolean conditions:")
var isTrue = true
var isFalse = false

if isTrue {
    print("isTrue is true")
}

if !isFalse {
    print("isFalse is false (negated)")
}

// Test 6: Complex nested structure
print("\n6. Complex nested structure:")
for (var outer = 1; outer <= 3; outer = outer + 1) {
    if outer == 2 {
        print("Skipping outer = 2")
    } else {
        print("Processing outer = ")
        print(outer)
        var inner = 1
        while inner <= 2 {
            print("  inner = ")
            print(inner)
            inner = inner + 1
        }
    }
}

// Test 7: Variable shadowing in nested scopes
print("\n7. Variable shadowing test:")
var name = "outer"
print("Outer name: ")
print(name)
{
    var name = "inner"
    print("Inner name: ")
    print(name)
    {
        var name = "innermost"
        print("Innermost name: ")
        print(name)
    }
    print("Back to inner name: ")
    print(name)
}
print("Back to outer name: ")
print(name)

print("\n=== Advanced Control Flow Tests Complete ===")