// Simple UI Application Test for MiniSwift
// This demonstrates basic UI components and layout

// Import UI framework (conceptual - will be handled by interpreter)
import MiniSwiftUI

// Define a simple UI application
struct SimpleUIApp {
    
    // Main application entry point
    func run() {
        print("Starting Simple UI Application...")
        
        // Create main window content
        let mainView = self.createMainView()
        
        print("Main view created: ", mainView)
        print("Note: This is a Mock UI backend - components are rendered to console")
        print("To see actual windows, GTK4 backend would need to be available")
        
        print("Simple UI Application completed successfully!")
    }
    
    // Create the main view hierarchy
    func createMainView() {
        // Title
        let titleText = Text("Welcome to MiniSwift UI!")
            .font(Font.system(size: 24))
            .foregroundColor(Color.blue)
        
        // Subtitle
        let subtitleText = Text("This is a simple UI application built with MiniSwift")
            .font(Font.system(size: 16))
            .foregroundColor(Color.gray)
        
        // Hello button
        let helloButton = Button("Hello")
            .background(Color.green)
            .foregroundColor("#000000")
            .padding()
        
        // World button
        let worldButton = Button("World")
            .background(Color.orange)
            .foregroundColor(Color.white)
            .padding()
        
        // Button section
        let buttonStack = HStack()
        buttonStack.addChild(helloButton)
        buttonStack.addChild(worldButton)
        
        // Features title
        let featuresTitle = Text("Features Demonstrated:")
            .font(Font.system(size: 18))
            .foregroundColor(Color.black)
        
        // Feature items
        let feature1 = Text("• Text widgets with styling")
            .font(Font.system(size: 14))
            .foregroundColor(Color.black)
        
        let feature2 = Text("• Button widgets with actions")
            .font(Font.system(size: 14))
            .foregroundColor(Color.black)
        
        let feature3 = Text("• VStack and HStack layouts")
            .font(Font.system(size: 14))
            .foregroundColor(Color.black)
        
        let feature4 = Text("• Color and padding support")
            .font(Font.system(size: 14))
            .foregroundColor(Color.black)
        
        // Info section
        let infoStack = VStack()
        infoStack.addChild(featuresTitle)
        infoStack.addChild(feature1)
        infoStack.addChild(feature2)
        infoStack.addChild(feature3)
        infoStack.addChild(feature4)
        
        // Footer
        let footerText = Text("Powered by MiniSwift UI System")
            .font(Font.system(size: 12))
            .foregroundColor(Color.black)
        
        // Main stack
        let mainStack = VStack()
        mainStack.addChild(titleText)
        mainStack.addChild(subtitleText)
        mainStack.addChild(buttonStack)
        mainStack.addChild(infoStack)
        mainStack.addChild(footerText)
        
        print("Main view created successfully!")
        return mainStack
    }
    
    // Helper function to show messages
    func showMessage(_ message: String) {
        print("[UI Message] ", message)
    }
}

// Application entry point
let app = SimpleUIApp()
app.run()

print("Simple UI Application completed.")