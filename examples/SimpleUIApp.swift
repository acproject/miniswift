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
        
        // Set up the application
        UIApplication.shared.setRootView(mainView)
        
        // Run the application
        UIApplication.shared.run()
    }
    
    // Create the main view hierarchy
    func createMainView() -> VStack {
        return VStack(spacing: 20) {
            // Title
            Text("Welcome to MiniSwift UI!")
                .font(.title)
                .foregroundColor(.blue)
            
            // Subtitle
            Text("This is a simple UI application built with MiniSwift")
                .font(.body)
                .foregroundColor(.gray)
            
            // Button section
            HStack(spacing: 15) {
                Button("Hello") {
                    print("Hello button clicked!")
                    showMessage("Hello from MiniSwift!")
                }
                .padding()
                .backgroundColor(.green)
                .cornerRadius(8)
                
                Button("World") {
                    print("World button clicked!")
                    showMessage("World says hi!")
                }
                .padding()
                .backgroundColor(.orange)
                .cornerRadius(8)
            }
            
            // Info section
            VStack(spacing: 10) {
                Text("Features Demonstrated:")
                    .font(.headline)
                    .foregroundColor(.black)
                
                Text("• Text widgets with styling")
                Text("• Button widgets with actions")
                Text("• VStack and HStack layouts")
                Text("• Color and padding support")
            }
            .padding()
            .backgroundColor(.lightGray)
            .cornerRadius(12)
            
            // Footer
            Text("Powered by MiniSwift UI System")
                .font(.caption)
                .foregroundColor(.darkGray)
        }
        .padding(30)
    }
    
    // Helper function to show messages
    func showMessage(_ message: String) {
        print("[UI Message] \(message)")
        // In a real implementation, this could show a dialog or update UI
    }
}

// Color extensions for convenience
extension Color {
    static var lightGray: Color {
        get {
            return Color(red: 0.9, green: 0.9, blue: 0.9, alpha: 1.0)
        }
    }
    static var darkGray: Color {
        get {
            return Color(red: 0.3, green: 0.3, blue: 0.3, alpha: 1.0)
        }
    }
}

// Font extensions
extension Font {
    static var title: Font {
        get {
            return Font(family: "System", size: 24, bold: true)
        }
    }
    static var headline: Font {
        get {
            return Font(family: "System", size: 18, bold: true)
        }
    }
    static var body: Font {
        get {
            return Font(family: "System", size: 16, bold: false)
        }
    }
    static var caption: Font {
        get {
            return Font(family: "System", size: 12, bold: false)
        }
    }
}

// Application entry point
let app = SimpleUIApp()
app.run()

print("Simple UI Application completed.")