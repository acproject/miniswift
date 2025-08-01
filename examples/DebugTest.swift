// Debug Test for GTK4 UI System
// This is a minimal test to debug the rendering issue

import MiniSwiftUI

struct DebugTest {
    func run() {
        // print("[DEBUG] Starting debug test...")
        
        // Create a single text widget
        // print("[DEBUG] Creating single text widget...")
        let singleText = Text("Hello GTK4! 中文测试")
        // print("[DEBUG] Single text widget created")
        
        // Set as root view
        // print("[DEBUG] Setting root view...")
        UIApplication.shared.setRootView(singleText)
        // print("[DEBUG] Root view set")
        
        // Run application
        // print("[DEBUG] Running application...")
        UIApplication.shared.run()
        // print("[DEBUG] Application finished")
    }
}

@main
func main() {
    print("[DEBUG] Starting main function...")
    let test = DebugTest()
    test.run()
    print("[DEBUG] Main function completed")
}

main()
print("[DEBUG] Debug test completed.")