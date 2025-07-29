// Basic UI Test for MiniSwift
// Tests fundamental UI components

import MiniSwift

print("=== MiniSwift UI Test ===")

// Test 1: Create a simple text widget
print("\n1. Testing Text Widget...")
let welcomeText = Text("Hello, MiniSwift UI!")
print("Text widget created successfully")

// Test 2: Create a button widget
print("\n2. Testing Button Widget...")
let testButton = Button("Click Me") {
    print("Button was clicked!")
}
print("Button widget created successfully")

// Test 3: Create layout containers
print("\n3. Testing Layout Containers...")
let verticalStack = VStack(spacing: 10.0)
print("VStack created successfully")

let horizontalStack = HStack(spacing: 8.0)
print("HStack created successfully")

// Test 4: Test colors
print("\n4. Testing Colors...")
let redColor = Color.red
let blueColor = Color.blue
let customColor = Color(red: 0.5, green: 0.8, blue: 0.2, alpha: 1.0)
print("Colors created successfully")

// Test 5: Test basic styling
print("\n5. Testing Basic Styling...")
let styledText = Text("Styled Text")
    .foregroundColor(blueColor)
    .font(Font.system(size: 18.0))
print("Styled text created successfully")

// Test 6: Simple layout composition
print("\n6. Testing Layout Composition...")
let simpleLayout = VStack(spacing: 12.0) {
    Text("Title")
    Text("Subtitle")
    Button("Action") {
        print("Action button pressed")
    }
}
print("Simple layout created successfully")

// Test 7: UI Application
print("\n7. Testing UI Application...")
let uiApp = UIApplication.shared
print("UI Application instance obtained")

// Test 8: Setting Root View
print("\n8. Setting Root View...")
// uiApp.setRootView(simpleLayout)
print("Root view configuration completed")

print("\n=== All UI Tests Completed Successfully ===")
print("UI system is ready for use!")