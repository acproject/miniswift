# MiniSwift UI Library

A SwiftUI-inspired UI framework for MiniSwift, built on top of GTK4.

## Overview

MiniSwift UI provides a declarative way to build user interfaces using a syntax similar to SwiftUI. It leverages GTK4 as the underlying UI toolkit while providing a modern, Swift-like API.

## Features

### Layout Containers
- **VStack**: Arranges views vertically
- **HStack**: Arranges views horizontally  
- **ZStack**: Overlays views on the Z-axis
- **Grid**: Two-dimensional grid layout

### Basic Components
- **Text**: Display text with styling options
- **Button**: Interactive button with customizable appearance
- **Image**: Display images from files or system icons
- **Rectangle**: Basic shape for backgrounds and dividers
- **Spacer**: Flexible spacing element
- **Divider**: Visual separator

### Styling System
- **Colors**: Predefined and custom colors
- **Fonts**: System fonts and custom typography
- **Modifiers**: Chainable view modifiers for styling

### View Modifiers
- `.frame()`: Set view size and constraints
- `.padding()`: Add spacing around views
- `.background()`: Set background color or view
- `.foregroundColor()`: Set text/icon color
- `.border()`: Add borders
- `.cornerRadius()`: Round corners
- `.shadow()`: Add drop shadows
- `.opacity()`: Set transparency

## Basic Usage

### Simple Text View
```swift
Text("Hello, MiniSwift!")
    .font(.title)
    .foregroundColor(.blue)
    .padding()
```

### Layout with VStack
```swift
VStack(spacing: 20) {
    Text("Welcome")
        .font(.largeTitle)
    
    Button("Get Started") {
        print("Button tapped!")
    }
    .background(.blue)
    .foregroundColor(.white)
    .cornerRadius(8)
}
```

### Complex Layout
```swift
HStack {
    VStack(alignment: .leading) {
        Text("Title")
            .font(.headline)
        Text("Subtitle")
            .font(.caption)
            .foregroundColor(.secondary)
    }
    
    Spacer()
    
    Image(systemName: "chevron.right")
        .foregroundColor(.gray)
}
.padding()
.background(.white)
.cornerRadius(12)
.shadow(radius: 2)
```

## Architecture

### Core Protocols

#### UIView
The base protocol for all UI components:
```swift
protocol UIView {
    var body: Self { get }
    func render() -> GTKWidget
}
```

#### ViewModifier
Protocol for creating reusable view modifications:
```swift
protocol ViewModifier {
    associatedtype Content: UIView
    associatedtype Body: UIView
    
    func body(content: Content) -> Body
}
```

### ViewBuilder
Result builder for composing multiple views:
```swift
@resultBuilder
struct ViewBuilder {
    static func buildBlock<Content: UIView>(_ content: Content) -> Content
    // Additional overloads for multiple views
}
```

## Styling

### Colors
```swift
// Predefined colors
.foregroundColor(.blue)
.background(.red)

// Custom colors
.foregroundColor(Color(red: 0.2, green: 0.4, blue: 0.8))
```

### Fonts
```swift
// System fonts
.font(.title)
.font(.body)
.font(.caption)

// Custom fonts
.font(.system(size: 18, weight: .bold))
.font(.custom("Helvetica", size: 16))
```

### Layout
```swift
// Frame sizing
.frame(width: 200, height: 100)
.frame(maxWidth: .infinity)

// Padding
.padding()
.padding(.horizontal, 20)
.padding(EdgeInsets(top: 10, leading: 15, bottom: 10, trailing: 15))
```

## Advanced Features

### Custom Components
```swift
struct CustomCard: UIView {
    let title: String
    let content: String
    
    var body: some UIView {
        VStack(alignment: .leading) {
            Text(title)
                .font(.headline)
            Text(content)
                .font(.body)
                .foregroundColor(.secondary)
        }
        .padding()
        .background(.white)
        .cornerRadius(12)
        .shadow(radius: 4)
    }
}
```

### Custom Modifiers
```swift
struct CardStyle: ViewModifier {
    func body(content: Content) -> some UIView {
        content
            .padding()
            .background(.white)
            .cornerRadius(12)
            .shadow(radius: 4)
    }
}

extension UIView {
    func cardStyle() -> some UIView {
        modifier(CardStyle())
    }
}
```

## GTK4 Integration

The UI library translates MiniSwift UI components to GTK4 widgets:

- **VStack/HStack** → GTK Box with orientation
- **ZStack** → GTK Overlay
- **Grid** → GTK Grid
- **Text** → GTK Label
- **Button** → GTK Button
- **Image** → GTK Image

## Examples

See `examples/UIExample.swift` for comprehensive examples including:
- Basic component usage
- Layout combinations
- Styling and modifiers
- Complex application layouts

## Future Enhancements

- [ ] Animation system
- [ ] State management (@State, @Binding)
- [ ] Navigation (NavigationView, NavigationLink)
- [ ] Lists and ForEach
- [ ] Gestures and interactions
- [ ] Custom drawing and shapes
- [ ] Accessibility support
- [ ] Theme system
- [ ] Performance optimizations

## Building and Running

1. Ensure GTK4 is installed on your system
2. Build the MiniSwift project with UI support
3. Import MiniSwift in your Swift files
4. Create your UI using the declarative syntax
5. Call `.render().show()` on your root view to display

## Contributing

Contributions are welcome! Please see the main project README for contribution guidelines.