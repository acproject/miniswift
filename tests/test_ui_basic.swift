// Basic UI Library Test
// Tests the fundamental UI components and layouts

import MiniSwift

// Test basic text component
struct SimpleTextTest: UIView {
    var body: some UIView {
        Text("Hello, MiniSwift UI!")
            .font(.title)
            .foregroundColor(.blue)
            .padding()
    }
}

// Test basic layout
struct BasicLayoutTest: UIView {
    var body: some UIView {
        VStack(spacing: 10) {
            Text("Title")
                .font(.headline)
            
            Text("This is a subtitle")
                .font(.body)
                .foregroundColor(.gray)
            
            HStack {
                Button("Cancel") {
                    print("Cancel tapped")
                }
                .foregroundColor(.red)
                
                Spacer()
                
                Button("OK") {
                    print("OK tapped")
                }
                .background(.blue)
                .foregroundColor(.white)
                .cornerRadius(8)
            }
        }
        .padding()
    }
}

// Test styling and modifiers
struct StylingTest: UIView {
    var body: some UIView {
        VStack {
            Rectangle()
                .fill(.blue)
                .frame(width: 100, height: 100)
                .cornerRadius(12)
                .shadow(radius: 5)
            
            Text("Styled Rectangle")
                .font(.caption)
        }
    }
}

// Test grid layout
struct GridTest: UIView {
    var body: some UIView {
        Grid(horizontalSpacing: 8, verticalSpacing: 8) {
            GridRow {
                Rectangle().fill(.red).frame(width: 50, height: 50)
                Rectangle().fill(.green).frame(width: 50, height: 50)
                Rectangle().fill(.blue).frame(width: 50, height: 50)
            }
            GridRow {
                Rectangle().fill(.orange).frame(width: 50, height: 50)
                Rectangle().fill(.purple).frame(width: 50, height: 50)
                Rectangle().fill(.pink).frame(width: 50, height: 50)
            }
        }
        .padding()
    }
}

// Main test application
struct TestApp: UIView {
    var body: some UIView {
        VStack(spacing: 20) {
            Text("MiniSwift UI Test Suite")
                .font(.largeTitle)
                .foregroundColor(.primary)
            
            Divider()
            
            SimpleTextTest()
            
            Divider()
            
            BasicLayoutTest()
            
            Divider()
            
            StylingTest()
            
            Divider()
            
            GridTest()
        }
        .padding()
        .background(.white)
    }
}

// Test execution
func runUITests() {
    print("Starting MiniSwift UI tests...")
    
    // Create test app
    let app = TestApp()
    
    // Render to GTK4 (this would actually display the UI)
    let widget = app.render()
    
    print("UI tests completed successfully!")
    print("Note: Actual GTK4 rendering requires GTK4 backend implementation")
}

// Run the tests
runUITests()