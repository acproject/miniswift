// MiniSwift UI Library Example
// Demonstrates basic UI components and layouts

import MiniSwift

/// Main application view
struct ContentView: UIView {
    var body: some UIView {
        VStack(spacing: 20) {
            // Header section
            Text("Welcome to MiniSwift UI")
                .font(.largeTitle)
                .foregroundColor(.primary)
                .padding()
            
            // Image section
            Image(systemName: "swift")
                .resizable()
                .aspectRatio(1.0, contentMode: .fit)
                .frame(width: 100, height: 100)
                .foregroundColor(.orange)
            
            // Button section
            HStack(spacing: 15) {
                Button("Primary Action") {
                    print("Primary button tapped!")
                }
                .background(.blue)
                .foregroundColor(.white)
                .cornerRadius(8)
                .padding(.horizontal)
                
                Button("Secondary") {
                    print("Secondary button tapped!")
                }
                .border(.gray, width: 1)
                .cornerRadius(8)
                .padding(.horizontal)
            }
            
            // Card-like content
            VStack(alignment: .leading, spacing: 10) {
                Text("Features")
                    .font(.headline)
                    .foregroundColor(.primary)
                
                VStack(alignment: .leading, spacing: 5) {
                    FeatureRow(icon: "checkmark.circle", title: "SwiftUI-like syntax")
                    FeatureRow(icon: "gear", title: "GTK4 backend")
                    FeatureRow(icon: "paintbrush", title: "Modern styling")
                    FeatureRow(icon: "layout", title: "Flexible layouts")
                }
            }
            .padding()
            .background(.gray.opacity(0.1))
            .cornerRadius(12)
            .padding(.horizontal)
            
            Spacer()
            
            // Footer
            Text("Built with MiniSwift UI Framework")
                .font(.caption)
                .foregroundColor(.secondary)
        }
        .padding()
    }
}

/// Feature row component
struct FeatureRow: UIView {
    let icon: String
    let title: String
    
    var body: some UIView {
        HStack(spacing: 10) {
            Image(systemName: icon)
                .foregroundColor(.green)
                .frame(width: 20, height: 20)
            
            Text(title)
                .font(.body)
            
            Spacer()
        }
    }
}

/// Grid layout example
struct GridExample: UIView {
    var body: some UIView {
        VStack {
            Text("Grid Layout Example")
                .font(.title)
                .padding()
            
            Grid(horizontalSpacing: 10, verticalSpacing: 10) {
                GridRow {
                    ColorBox(color: .red, label: "Red")
                    ColorBox(color: .green, label: "Green")
                    ColorBox(color: .blue, label: "Blue")
                }
                
                GridRow {
                    ColorBox(color: .orange, label: "Orange")
                    ColorBox(color: .purple, label: "Purple")
                    ColorBox(color: .pink, label: "Pink")
                }
            }
            .padding()
        }
    }
}

/// Color box component for grid
struct ColorBox: UIView {
    let color: Color
    let label: String
    
    var body: some UIView {
        VStack {
            Rectangle()
                .fill(color)
                .frame(width: 60, height: 60)
                .cornerRadius(8)
            
            Text(label)
                .font(.caption)
                .foregroundColor(.primary)
        }
    }
}

/// ZStack overlay example
struct OverlayExample: UIView {
    var body: some UIView {
        ZStack {
            // Background
            Rectangle()
                .fill(.blue.opacity(0.3))
                .frame(width: 200, height: 200)
                .cornerRadius(20)
            
            // Overlay content
            VStack {
                Image(systemName: "star.fill")
                    .foregroundColor(.yellow)
                    .font(.largeTitle)
                
                Text("Overlay")
                    .font(.headline)
                    .foregroundColor(.white)
            }
        }
        .shadow(color: .black.opacity(0.3), radius: 10, x: 0, y: 5)
    }
}

/// Complex layout example
struct ComplexLayoutExample: UIView {
    var body: some UIView {
        VStack(spacing: 0) {
            // Header
            HStack {
                Text("MiniSwift UI")
                    .font(.title)
                    .foregroundColor(.white)
                
                Spacer()
                
                Button("Menu") {
                    print("Menu tapped")
                }
                .foregroundColor(.white)
            }
            .padding()
            .background(.blue)
            
            // Content area
            HStack(spacing: 0) {
                // Sidebar
                VStack(alignment: .leading) {
                    Text("Navigation")
                        .font(.headline)
                        .padding(.bottom)
                    
                    VStack(alignment: .leading, spacing: 8) {
                        NavigationItem(title: "Home", isSelected: true)
                        NavigationItem(title: "Settings", isSelected: false)
                        NavigationItem(title: "About", isSelected: false)
                    }
                    
                    Spacer()
                }
                .frame(width: 150)
                .padding()
                .background(.gray.opacity(0.1))
                
                // Main content
                VStack {
                    Text("Main Content Area")
                        .font(.title2)
                        .padding()
                    
                    Text("This is where the main application content would be displayed.")
                        .padding()
                    
                    Spacer()
                }
                .frame(maxWidth: .infinity)
            }
        }
    }
}

/// Navigation item component
struct NavigationItem: UIView {
    let title: String
    let isSelected: Bool
    
    var body: some UIView {
        Text(title)
            .foregroundColor(isSelected ? .blue : .primary)
            .padding(.vertical, 4)
            .background(isSelected ? .blue.opacity(0.1) : .clear)
            .cornerRadius(4)
    }
}

// Example usage:
// let app = ContentView()
// app.render().show()