// MiniSwift UI Extension Demo
// This example demonstrates the new UI extension capabilities
import MiniSwift
// Define a simple counter app
struct CounterApp {
    var count: Int = 0
    
    func body() {
        return VStack {
            // Title
            Text("MiniSwift Counter App")
                .font(Font.system(size: 28))
                .foregroundColor(Color.blue)
                .padding(20)
            
            // Counter display
            Text("Count: " + String(count))
                .font(Font.system(size: 24))
                .foregroundColor(Color.white)
                .padding(16)
                .background(Color.black)
            
            // Button row
            HStack {
                Button("Decrease") {
                    count = count - 1
                    print("Count decreased to: " + String(count))
                }
                .background(Color.red)
                .foregroundColor(Color.white)
                .padding(12)
                
                Button("Increase") {
                    count = count + 1
                    print("Count increased to: " + String(count))
                }
                .background(Color.green)
                .foregroundColor(Color.white)
                .padding(12)
            }
            .padding(16)
            
            // Reset button
            Button("Reset") {
                count = 0
                print("Count reset to 0")
            }
            .background(Color.blue)
            .foregroundColor(Color.white)
            .padding(10)
        }
        .background(Color.white)
        .padding(24)
    }
}

// Create and run the app
print("Starting MiniSwift UI Extension Demo...")

let app = CounterApp()
let mainView = app.body()

// Set up the UI application
UIApplication.shared.setRootView(mainView)
print("UI Application configured successfully")

// Run the application
print("Running UI Application...")
UIApplication.shared.run()