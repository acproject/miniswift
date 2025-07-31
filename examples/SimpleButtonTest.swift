// Simple Button Test for MiniSwift UI
// Testing basic UI component creation without chaining

import MiniSwift

func testBasicUI() {
    print("Creating basic UI components...")
    
    // Create simple text widget
    let text = Text("Hello World")
    print("Text created:", text)
    
    // Create simple button widget
    let button = Button("Click Me")
    print("Button created:", button)
    
    // Create VStack container
    let stack = VStack(spacing: 10)
    print("VStack created:", stack)
    
    print("Basic UI test completed!")
}

// Run the test
testBasicUI()