// Button and Layout Test for MiniSwift
// This tests if buttons and layout components work correctly

import MiniSwiftUI

print("Starting Button and Layout Test...")

// Create a simple text widget
let titleText = Text("Button Test")
print("Title text created")

// Create a button widget
let testButton = Button("Test Button")
print("Button created")

// Create an HStack to hold the button
let buttonStack = HStack()
buttonStack.addChild(testButton)
print("HStack created and button added")

// Create a VStack to hold everything
let mainStack = VStack()
mainStack.addChild(titleText)
mainStack.addChild(buttonStack)
print("VStack created and components added")

// Set up and run the UI application
UIApplication.shared.setRootView(mainStack)
print("Root view set")

UIApplication.shared.run()
print("Application finished")