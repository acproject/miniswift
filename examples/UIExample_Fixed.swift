// MiniSwift UI Library Example - Fixed for current interpreter
// Demonstrates basic UI components and layouts compatible with current implementation

import MiniSwift

// Simple function to create main content view
func createContentView() {
    let headerText = Text("Welcome to MiniSwift UI")
        .font(Font.system(size: 24))
        .foregroundColor(Color.blue)
        .padding()
    
    let primaryButton = Button("Primary Action")
        .background(Color.blue)
        .foregroundColor(Color.white)
        .padding()
        
    let secondaryButton = Button("Secondary")
        .background(Color.green)
        .foregroundColor(Color.white)
        .padding()
    
    let buttonStack = HStack(spacing: 15) {
        primaryButton
        secondaryButton
    }
    
    let featuresText = Text("Features")
        .font(Font.system(size: 18))
        .foregroundColor(Color.black)
    
    let mainStack = VStack(spacing: 20) {
        headerText
        buttonStack
        featuresText
    }
    
    print("Content view created successfully!")
    return mainStack
}

// Function to create a feature row
func createFeatureRow(title: String) {
    let featureText = Text(title)
        .font(Font.system(size: 14))
        .foregroundColor(Color.black)
    
    print("Feature row created: ", title)
    return featureText
}

// Function to create a color box
func createColorBox(colorName: String) {
    let colorText = Text(colorName)
        .font(Font.system(size: 12))
        .foregroundColor(Color.black)
    
    print("Color box created: ", colorName)
    return colorText
}

// Function to demonstrate basic layouts
func demonstrateLayouts() {
    print("=== Basic Layout Demo ===")
    
    // Create some feature rows
    let feature1 = createFeatureRow(title: "SwiftUI-like syntax")
    let feature2 = createFeatureRow(title: "GTK4 backend")
    let feature3 = createFeatureRow(title: "Modern styling")
    let feature4 = createFeatureRow(title: "Flexible layouts")
    
    // Create color boxes
    let redBox = createColorBox(colorName: "Red")
    let greenBox = createColorBox(colorName: "Green")
    let blueBox = createColorBox(colorName: "Blue")
    
    // Create a simple grid-like structure using VStack and HStack
    let colorRow = HStack(spacing: 10) {
        redBox
        greenBox
        blueBox
    }
    
    let featuresColumn = VStack(spacing: 5) {
        feature1
        feature2
        feature3
        feature4
    }
    
    print("Layout demonstration completed!")
}

// Function to create navigation items
func createNavigationItem(title: String, isSelected: Bool) {
    let navText = Text(title)
        .font(Font.system(size: 14))
        .foregroundColor(isSelected ? Color.blue : Color.black)
    
    print("Navigation item created: ", title, " (selected: ", isSelected, ")")
    return navText
}

// Function to demonstrate complex layout
func demonstrateComplexLayout() {
    print("=== Complex Layout Demo ===")
    
    // Header
    let headerTitle = Text("MiniSwift UI")
        .font(Font.system(size: 20))
        .foregroundColor(Color.white)
    
    let menuButton = Button("Menu")
        .foregroundColor(Color.white)
    
    let header = HStack(spacing: 0) {
        headerTitle
        menuButton
    }
    
    // Navigation
    let navTitle = Text("Navigation")
        .font(Font.system(size: 16))
    
    let homeItem = createNavigationItem(title: "Home", isSelected: true)
    let settingsItem = createNavigationItem(title: "Settings", isSelected: false)
    let aboutItem = createNavigationItem(title: "About", isSelected: false)
    
    let navigation = VStack(spacing: 8) {
        navTitle
        homeItem
        settingsItem
        aboutItem
    }
    
    // Main content
    let contentTitle = Text("Main Content Area")
        .font(Font.system(size: 18))
    
    let contentDescription = Text("This is where the main application content would be displayed.")
        .font(Font.system(size: 14))
    
    let mainContent = VStack(spacing: 10) {
        contentTitle
        contentDescription
    }
    
    print("Complex layout demonstration completed!")
}

// Main application function
func runUIExample() {
    print("Starting MiniSwift UI Example...")
    print("")
    
    // Create main content
    let mainContent = createContentView()
    print("Main content: ", mainContent)
    print("")
    
    // Demonstrate layouts
    demonstrateLayouts()
    print("")
    
    // Demonstrate complex layout
    demonstrateComplexLayout()
    print("")
    
    print("MiniSwift UI Example completed successfully!")
    print("Built with MiniSwift UI Framework")
}

// Run the example
runUIExample()