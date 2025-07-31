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
        
        // Set up the UI application
        UIApplication.shared.setRootView(mainView)
        print("UI Application configured successfully")
        
        // Run the application
        print("Running UI Application...")
        UIApplication.shared.run()
    }
    
    // Create the main view hierarchy
    func createMainView() -> VStack {
        print("[DEBUG] Starting createMainView...")
        
        // Title
        print("[DEBUG] Creating title text...")
        let titleText = Text("Welcome to MiniSwift UI!")
            .font(Font.system(size: 24))
            .foregroundColor(Color.blue)
        print("[DEBUG] Title text created successfully")
        
        // Subtitle
        print("[DEBUG] Creating subtitle text...")
        let subtitleText = Text("This is a simple UI application built with MiniSwift")
            .font(Font.system(size: 16))
            .foregroundColor(Color.gray)
        print("[DEBUG] Subtitle text created successfully")
        
        // Hello button
        print("[DEBUG] Creating hello button...")
        let helloButton = Button("Hello")
            .background(Color.green)
            .foregroundColor("#000000")
            .padding()
        print("[DEBUG] Hello button created successfully")
        
        // World button
        print("[DEBUG] Creating world button...")
        let worldButton = Button("World")
            .background(Color.orange)
            .foregroundColor(Color.white)
            .padding()
        print("[DEBUG] World button created successfully")
        
        // Button section
        print("[DEBUG] Creating button stack...")
        let buttonStack = HStack()
        print("[DEBUG] Adding hello button to stack...")
        buttonStack.addChild(helloButton)
        print("[DEBUG] Adding world button to stack...")
        buttonStack.addChild(worldButton)
        print("[DEBUG] Button stack created successfully")
        
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
print("Starting MiniSwift Simple UI Application Demo...")

@main
func main() {
let app = SimpleUIApp()
app.run()
}

main()


print("Simple UI Application completed.")