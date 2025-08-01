// Minimal GTK4 Layout Test - Swift Version
// Testing VStack and HStack layouts

import MiniSwift

func createAndRunApp() {
    print("Starting Minimal GTK4 Layout Test...")
    
    // Create UI components
    let titleText = Text("GTK4 Layout Test")
    let descText = Text("Testing VStack and HStack layouts")
    
    let button1 = Button("Button 1")
    let button2 = Button("Button 2")
    
    let statusText = Text("Status: Layout components created")
    
    // Create layout containers
    let buttonStack = HStack(spacing: 15) // horizontal stack with 15px spacing
    let mainStack = VStack(spacing: 20)   // vertical stack with 20px spacing
    
    // Add buttons to horizontal stack
    buttonStack.addChild(button1)
    buttonStack.addChild(button2)
    
    // Add all components to main vertical stack
    mainStack.addChild(titleText)
    mainStack.addChild(descText)
    mainStack.addChild(buttonStack)
    mainStack.addChild(statusText)
    
    print("UI layout components created successfully")
    
    // Set the main view and run the application
    print("Starting UI application with layout...")
    UIApplication.shared.setRootView(mainStack)
    UIApplication.shared.run()
    
    print("Application finished successfully")
}

// Run the app
createAndRunApp()