// MiniSwift UI Library - Style and Appearance Types
// Inspired by SwiftUI styling system

/// Color representation
public struct Color {
    private let red: CGFloat
    private let green: CGFloat
    private let blue: CGFloat
    private let alpha: CGFloat
    
    public init(red: CGFloat, green: CGFloat, blue: CGFloat, opacity: CGFloat = 1.0) {
        self.red = red
        self.green = green
        self.blue = blue
        self.alpha = opacity
    }
    
    // Predefined colors
    public static let clear = Color(red: 0, green: 0, blue: 0, opacity: 0)
    public static let black = Color(red: 0, green: 0, blue: 0)
    public static let white = Color(red: 1, green: 1, blue: 1)
    public static let gray = Color(red: 0.5, green: 0.5, blue: 0.5)
    public static let red = Color(red: 1, green: 0, blue: 0)
    public static let green = Color(red: 0, green: 1, blue: 0)
    public static let blue = Color(red: 0, green: 0, blue: 1)
    public static let orange = Color(red: 1, green: 0.5, blue: 0)
    public static let yellow = Color(red: 1, green: 1, blue: 0)
    public static let pink = Color(red: 1, green: 0.75, blue: 0.8)
    public static let purple = Color(red: 0.5, green: 0, blue: 0.5)
    public static let primary = Color(red: 0, green: 0.5, blue: 1)
    public static let secondary = Color(red: 0.5, green: 0.5, blue: 0.5)
    
    // Convert to GTK4 color format
    func toGTK() -> String {
        return "rgba(\(Int(red * 255)), \(Int(green * 255)), \(Int(blue * 255)), \(alpha))"
    }
    
    // Color modifiers
    public func opacity(_ opacity: CGFloat) -> Color {
        return Color(red: red, green: green, blue: blue, opacity: opacity)
    }
}

/// Font representation
public struct Font {
    private let name: String
    private let size: CGFloat
    private let weight: Weight
    private let design: Design
    
    private init(name: String, size: CGFloat, weight: Weight = .regular, design: Design = .default) {
        self.name = name
        self.size = size
        self.weight = weight
        self.design = design
    }
    
    // Font weights
    public enum Weight {
        case ultraLight
        case thin
        case light
        case regular
        case medium
        case semibold
        case bold
        case heavy
        case black
        
        func toGTK() -> String {
            switch self {
            case .ultraLight: return "ultralight"
            case .thin: return "thin"
            case .light: return "light"
            case .regular: return "normal"
            case .medium: return "medium"
            case .semibold: return "semibold"
            case .bold: return "bold"
            case .heavy: return "heavy"
            case .black: return "black"
            }
        }
    }
    
    // Font designs
    public enum Design {
        case `default`
        case serif
        case rounded
        case monospaced
        
        func toGTK() -> String {
            switch self {
            case .default: return "sans-serif"
            case .serif: return "serif"
            case .rounded: return "sans-serif"
            case .monospaced: return "monospace"
            }
        }
    }
    
    // System fonts
    public static let largeTitle = Font(name: "System", size: 34, weight: .regular)
    public static let title = Font(name: "System", size: 28, weight: .regular)
    public static let title2 = Font(name: "System", size: 22, weight: .regular)
    public static let title3 = Font(name: "System", size: 20, weight: .regular)
    public static let headline = Font(name: "System", size: 17, weight: .semibold)
    public static let body = Font(name: "System", size: 17, weight: .regular)
    public static let callout = Font(name: "System", size: 16, weight: .regular)
    public static let subheadline = Font(name: "System", size: 15, weight: .regular)
    public static let footnote = Font(name: "System", size: 13, weight: .regular)
    public static let caption = Font(name: "System", size: 12, weight: .regular)
    public static let caption2 = Font(name: "System", size: 11, weight: .regular)
    
    // Custom font creation
    public static func system(size: CGFloat, weight: Weight = .regular, design: Design = .default) -> Font {
        return Font(name: "System", size: size, weight: weight, design: design)
    }
    
    public static func custom(_ name: String, size: CGFloat) -> Font {
        return Font(name: name, size: size)
    }
    
    // Font modifiers
    public func weight(_ weight: Weight) -> Font {
        return Font(name: name, size: size, weight: weight, design: design)
    }
    
    public func italic() -> Font {
        // Implementation for italic font
        return self
    }
    
    // Convert to GTK4 font format
    func toGTK() -> String {
        return "\(design.toGTK()) \(weight.toGTK()) \(Int(size))px"
    }
}

/// Text alignment options
public enum TextAlignment {
    case leading
    case center
    case trailing
    
    func toGTK() -> String {
        switch self {
        case .leading: return "start"
        case .center: return "center"
        case .trailing: return "end"
        }
    }
}

/// Content mode for images
public enum ContentMode {
    case fit
    case fill
    
    func toGTK() -> String {
        switch self {
        case .fit: return "contain"
        case .fill: return "cover"
        }
    }
}

/// Shadow configuration
public struct Shadow {
    let color: Color
    let radius: CGFloat
    let x: CGFloat
    let y: CGFloat
    
    public init(color: Color = .black, radius: CGFloat, x: CGFloat = 0, y: CGFloat = 0) {
        self.color = color
        self.radius = radius
        self.x = x
        self.y = y
    }
}

/// Border configuration
public struct Border {
    let color: Color
    let width: CGFloat
    
    public init(_ color: Color, width: CGFloat = 1) {
        self.color = color
        self.width = width
    }
}

/// Corner radius configuration
public struct CornerRadius {
    let radius: CGFloat
    
    public init(_ radius: CGFloat) {
        self.radius = radius
    }
}