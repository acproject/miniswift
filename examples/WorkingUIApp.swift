// Working UI Application Test
// Testing complete UI app with root view and application run

import MiniSwift

func createAndRunApp() {
    print("Creating UI application...")
    
    // Create UI components
    let welcomeText = Text("Welcome to MiniSwift!")
    let clickButton = Button("Click Me")
    
    // Create main layout
    let mainStack = VStack(spacing: 20)
    
    print("UI components created successfully")
    print("Text:", welcomeText)
    print("Button:", clickButton)
    print("VStack:", mainStack)
    
    // Set up the application
    UIApplication.shared.setRootView(mainStack)
    print("Root view set successfully")
    
    // Run the application
    print("Starting UI application...")
    UIApplication.shared.run()
    print("UI application finished")
}

// Run the app
createAndRunApp()