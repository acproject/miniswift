// Simplified MiniSwift UI Example
// Compatible with current interpreter capabilities

import MiniSwift

// Simple function to create a basic UI
func createSimpleUI() {
    // Create basic UI components
    let welcomeText = Text("Welcome to MiniSwift UI")
        .font(Font.system(size: 24))
        .foregroundColor(Color.blue)
        .padding()
    
    let primaryButton = Button("Click Me")
        .background(Color.blue)
        .foregroundColor(Color.white)
        .padding()
    
    let secondaryButton = Button("Secondary")
        .background(Color.green)
        .foregroundColor(Color.white)
        .padding()
    
    // Create layout containers
    let buttonStack = HStack(spacing: 10) {
        primaryButton
        secondaryButton
    }
    
    let mainStack = VStack(spacing: 20) {
        welcomeText
        buttonStack
    }
    
    print("UI components created successfully!")
    print("Main UI structure: ", mainStack)
}

// Simple app structure without complex protocols
func runSimpleApp() {
    print("Starting Simple MiniSwift UI App...")
    createSimpleUI()
    print("App completed successfully!")
}

// Run the app
runSimpleApp()