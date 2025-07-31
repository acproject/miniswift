// Simple test for MiniSwift UI
import MiniSwiftUI

struct SimpleTest {
    func run() {
        print("Starting simple test...")
        
        // Create a simple text widget
        let text = Text("Hello World!")
        print("Text widget created")
        
        // Set up the UI application
        UIApplication.shared.setRootView(text)
        print("Root view set")
        
        // Run the application
        print("Running application...")
        UIApplication.shared.run()
    }
}

@main
func main() {
    let test = SimpleTest()
    test.run()
}

main()
print("Test completed.")