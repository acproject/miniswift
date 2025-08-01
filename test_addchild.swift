// Test addChild functionality
import MiniSwiftUI

// Create a simple VStack with one child
let text1 = Text("Hello")
let vstack = VStack(spacing: 10)
    .addChild(text1)

print("VStack with child created successfully")
print("Test completed.")