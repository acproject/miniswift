// MiniSwift UI Library - Base View Protocol
// Inspired by SwiftUI architecture

/// Base protocol for all UI views
public protocol UIView {
    /// The body of the view
    var body: Self { get }
    
    /// Render the view to GTK4 widget
    func render() -> GTKWidget
}

/// Base GTK4 widget wrapper
public class GTKWidget {
    // This will be implemented with actual GTK4 bindings
    public init() {}
    
    public func addChild(_ child: GTKWidget) {
        // GTK4 container add child implementation
    }
    
    public func setSize(width: Int, height: Int) {
        // GTK4 widget size setting
    }
    
    public func show() {
        // GTK4 widget show implementation
    }
}

/// View modifier protocol
public protocol ViewModifier {
    associatedtype Content: UIView
    associatedtype Body: UIView
    
    func body(content: Content) -> Body
}

/// Extension to add modifier support to UIView
extension UIView {
    public func modifier<M: ViewModifier>(_ modifier: M) -> some UIView where M.Content == Self {
        return ModifiedView(content: self, modifier: modifier)
    }
}

/// Modified view wrapper
public struct ModifiedView<Content: UIView, Modifier: ViewModifier>: UIView where Modifier.Content == Content {
    let content: Content
    let modifier: Modifier
    
    public var body: Self {
        return self
    }
    
    public func render() -> GTKWidget {
        return modifier.body(content: content).render()
    }
}