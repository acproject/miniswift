// MiniSwift UI Library - Geometry and Alignment Types
// Inspired by SwiftUI geometry system

/// Core geometric value type
public typealias CGFloat = Double

/// 2D size structure
public struct CGSize {
    public let width: CGFloat
    public let height: CGFloat
    
    public init(width: CGFloat, height: CGFloat) {
        self.width = width
        self.height = height
    }
    
    public static let zero = CGSize(width: 0, height: 0)
}

/// 2D point structure
public struct CGPoint {
    public let x: CGFloat
    public let y: CGFloat
    
    public init(x: CGFloat, y: CGFloat) {
        self.x = x
        self.y = y
    }
    
    public static let zero = CGPoint(x: 0, y: 0)
}

/// Rectangle structure
public struct CGRect {
    public let origin: CGPoint
    public let size: CGSize
    
    public init(origin: CGPoint, size: CGSize) {
        self.origin = origin
        self.size = size
    }
    
    public init(x: CGFloat, y: CGFloat, width: CGFloat, height: CGFloat) {
        self.origin = CGPoint(x: x, y: y)
        self.size = CGSize(width: width, height: height)
    }
    
    public static let zero = CGRect(origin: .zero, size: .zero)
}

/// Horizontal alignment options
public enum HorizontalAlignment {
    case leading
    case center
    case trailing
    
    // Convert to GTK4 alignment
    func toGTK() -> String {
        switch self {
        case .leading: return "start"
        case .center: return "center"
        case .trailing: return "end"
        }
    }
}

/// Vertical alignment options
public enum VerticalAlignment {
    case top
    case center
    case bottom
    
    // Convert to GTK4 alignment
    func toGTK() -> String {
        switch self {
        case .top: return "start"
        case .center: return "center"
        case .bottom: return "end"
        }
    }
}

/// Combined alignment
public struct Alignment {
    public let horizontal: HorizontalAlignment
    public let vertical: VerticalAlignment
    
    public init(horizontal: HorizontalAlignment, vertical: VerticalAlignment) {
        self.horizontal = horizontal
        self.vertical = vertical
    }
    
    // Predefined alignments
    public static let center = Alignment(horizontal: .center, vertical: .center)
    public static let leading = Alignment(horizontal: .leading, vertical: .center)
    public static let trailing = Alignment(horizontal: .trailing, vertical: .center)
    public static let top = Alignment(horizontal: .center, vertical: .top)
    public static let bottom = Alignment(horizontal: .center, vertical: .bottom)
    public static let topLeading = Alignment(horizontal: .leading, vertical: .top)
    public static let topTrailing = Alignment(horizontal: .trailing, vertical: .top)
    public static let bottomLeading = Alignment(horizontal: .leading, vertical: .bottom)
    public static let bottomTrailing = Alignment(horizontal: .trailing, vertical: .bottom)
}

/// Edge insets
public struct EdgeInsets {
    public let top: CGFloat
    public let leading: CGFloat
    public let bottom: CGFloat
    public let trailing: CGFloat
    
    public init(top: CGFloat, leading: CGFloat, bottom: CGFloat, trailing: CGFloat) {
        self.top = top
        self.leading = leading
        self.bottom = bottom
        self.trailing = trailing
    }
    
    public init(_ all: CGFloat) {
        self.init(top: all, leading: all, bottom: all, trailing: all)
    }
    
    public static let zero = EdgeInsets(0)
}

/// Edge set for specifying which edges to affect
public struct Edge {
    public static let top = Edge()
    public static let leading = Edge()
    public static let bottom = Edge()
    public static let trailing = Edge()
    
    public struct Set {
        public static let all = Set()
        public static let horizontal = Set()
        public static let vertical = Set()
        public static let top = Set()
        public static let leading = Set()
        public static let bottom = Set()
        public static let trailing = Set()
    }
}

/// Unit point for specifying relative positions
public struct UnitPoint {
    public let x: CGFloat
    public let y: CGFloat
    
    public init(x: CGFloat, y: CGFloat) {
        self.x = x
        self.y = y
    }
    
    // Predefined unit points
    public static let zero = UnitPoint(x: 0, y: 0)
    public static let center = UnitPoint(x: 0.5, y: 0.5)
    public static let leading = UnitPoint(x: 0, y: 0.5)
    public static let trailing = UnitPoint(x: 1, y: 0.5)
    public static let top = UnitPoint(x: 0.5, y: 0)
    public static let bottom = UnitPoint(x: 0.5, y: 1)
    public static let topLeading = UnitPoint(x: 0, y: 0)
    public static let topTrailing = UnitPoint(x: 1, y: 0)
    public static let bottomLeading = UnitPoint(x: 0, y: 1)
    public static let bottomTrailing = UnitPoint(x: 1, y: 1)
}