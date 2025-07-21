// Simple Protocol System Test
// This file tests the basic protocol declaration functionality

// Define a simple protocol
protocol Drawable {
    func draw()
}

// Define a protocol with inheritance
protocol Shape: Drawable {
    func area() -> Double
}

// Test basic protocol declarations
print("Protocol system test completed")