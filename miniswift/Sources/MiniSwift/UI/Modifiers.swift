// MiniSwift UI Library - View Modifiers
// Inspired by SwiftUI modifier system

/// Frame modifier for setting view size
public struct FrameModifier: ViewModifier {
    let width: CGFloat?
    let height: CGFloat?
    let minWidth: CGFloat?
    let maxWidth: CGFloat?
    let minHeight: CGFloat?
    let maxHeight: CGFloat?
    let alignment: Alignment
    
    public func body(content: Content) -> some UIView {
        return FramedView(content: content, modifier: self)
    }
}

public struct FramedView<Content: UIView>: UIView {
    let content: Content
    let modifier: FrameModifier
    
    public var body: Self {
        return self
    }
    
    public func render() -> GTKWidget {
        let widget = content.render()
        // Apply frame constraints to GTK4 widget
        if let width = modifier.width {
            widget.setSize(width: Int(width), height: Int(modifier.height ?? 0))
        }
        return widget
    }
}

/// Padding modifier
public struct PaddingModifier: ViewModifier {
    let insets: EdgeInsets
    
    public func body(content: Content) -> some UIView {
        return PaddedView(content: content, insets: insets)
    }
}

public struct PaddedView<Content: UIView>: UIView {
    let content: Content
    let insets: EdgeInsets
    
    public var body: Self {
        return self
    }
    
    public func render() -> GTKWidget {
        let container = GTKWidget()
        let contentWidget = content.render()
        
        // Apply padding using GTK4 margins
        // contentWidget.setMargin(top: insets.top, left: insets.leading, 
        //                        bottom: insets.bottom, right: insets.trailing)
        
        container.addChild(contentWidget)
        return container
    }
}

/// Background modifier
public struct BackgroundModifier<Background: UIView>: ViewModifier {
    let background: Background
    let alignment: Alignment
    
    public func body(content: Content) -> some UIView {
        return BackgroundView(content: content, background: background, alignment: alignment)
    }
}

public struct BackgroundView<Content: UIView, Background: UIView>: UIView {
    let content: Content
    let background: Background
    let alignment: Alignment
    
    public var body: Self {
        return self
    }
    
    public func render() -> GTKWidget {
        let container = GTKWidget()
        
        // Add background first
        let backgroundWidget = background.render()
        container.addChild(backgroundWidget)
        
        // Add content on top
        let contentWidget = content.render()
        container.addChild(contentWidget)
        
        return container
    }
}

/// Foreground color modifier
public struct ForegroundColorModifier: ViewModifier {
    let color: Color
    
    public func body(content: Content) -> some UIView {
        return ColoredView(content: content, color: color)
    }
}

public struct ColoredView<Content: UIView>: UIView {
    let content: Content
    let color: Color
    
    public var body: Self {
        return self
    }
    
    public func render() -> GTKWidget {
        let widget = content.render()
        // Apply color to GTK4 widget
        // widget.setForegroundColor(color.toGTK())
        return widget
    }
}

/// Border modifier
public struct BorderModifier: ViewModifier {
    let color: Color
    let width: CGFloat
    
    public func body(content: Content) -> some UIView {
        return BorderedView(content: content, color: color, width: width)
    }
}

public struct BorderedView<Content: UIView>: UIView {
    let content: Content
    let color: Color
    let width: CGFloat
    
    public var body: Self {
        return self
    }
    
    public func render() -> GTKWidget {
        let widget = content.render()
        // Apply border to GTK4 widget
        // widget.setBorder(color: color.toGTK(), width: width)
        return widget
    }
}

/// Corner radius modifier
public struct CornerRadiusModifier: ViewModifier {
    let radius: CGFloat
    
    public func body(content: Content) -> some UIView {
        return RoundedView(content: content, radius: radius)
    }
}

public struct RoundedView<Content: UIView>: UIView {
    let content: Content
    let radius: CGFloat
    
    public var body: Self {
        return self
    }
    
    public func render() -> GTKWidget {
        let widget = content.render()
        // Apply corner radius to GTK4 widget
        // widget.setCornerRadius(radius)
        return widget
    }
}

/// Shadow modifier
public struct ShadowModifier: ViewModifier {
    let color: Color
    let radius: CGFloat
    let x: CGFloat
    let y: CGFloat
    
    public func body(content: Content) -> some UIView {
        return ShadowedView(content: content, color: color, radius: radius, x: x, y: y)
    }
}

public struct ShadowedView<Content: UIView>: UIView {
    let content: Content
    let color: Color
    let radius: CGFloat
    let x: CGFloat
    let y: CGFloat
    
    public var body: Self {
        return self
    }
    
    public func render() -> GTKWidget {
        let widget = content.render()
        // Apply shadow to GTK4 widget
        // widget.setShadow(color: color.toGTK(), radius: radius, offsetX: x, offsetY: y)
        return widget
    }
}

/// Extension to add modifiers to UIView
extension UIView {
    /// Set frame size and constraints
    public func frame(width: CGFloat? = nil, height: CGFloat? = nil, alignment: Alignment = .center) -> some UIView {
        let modifier = FrameModifier(width: width, height: height, minWidth: nil, maxWidth: nil, 
                                   minHeight: nil, maxHeight: nil, alignment: alignment)
        return modifier.body(content: self)
    }
    
    public func frame(minWidth: CGFloat? = nil, idealWidth: CGFloat? = nil, maxWidth: CGFloat? = nil,
                     minHeight: CGFloat? = nil, idealHeight: CGFloat? = nil, maxHeight: CGFloat? = nil,
                     alignment: Alignment = .center) -> some UIView {
        let modifier = FrameModifier(width: idealWidth, height: idealHeight, 
                                   minWidth: minWidth, maxWidth: maxWidth,
                                   minHeight: minHeight, maxHeight: maxHeight, 
                                   alignment: alignment)
        return modifier.body(content: self)
    }
    
    /// Add padding
    public func padding(_ insets: EdgeInsets) -> some UIView {
        let modifier = PaddingModifier(insets: insets)
        return modifier.body(content: self)
    }
    
    public func padding(_ edges: Edge.Set = .all, _ length: CGFloat? = nil) -> some UIView {
        let defaultPadding: CGFloat = length ?? 16
        let insets = EdgeInsets(defaultPadding)
        return padding(insets)
    }
    
    /// Set background
    public func background<Background: UIView>(_ background: Background, alignment: Alignment = .center) -> some UIView {
        let modifier = BackgroundModifier(background: background, alignment: alignment)
        return modifier.body(content: self)
    }
    
    public func background(_ color: Color) -> some UIView {
        return background(Rectangle().fill(color))
    }
    
    /// Set foreground color
    public func foregroundColor(_ color: Color) -> some UIView {
        let modifier = ForegroundColorModifier(color: color)
        return modifier.body(content: self)
    }
    
    /// Add border
    public func border(_ color: Color, width: CGFloat = 1) -> some UIView {
        let modifier = BorderModifier(color: color, width: width)
        return modifier.body(content: self)
    }
    
    /// Add corner radius
    public func cornerRadius(_ radius: CGFloat) -> some UIView {
        let modifier = CornerRadiusModifier(radius: radius)
        return modifier.body(content: self)
    }
    
    /// Add shadow
    public func shadow(color: Color = .black, radius: CGFloat, x: CGFloat = 0, y: CGFloat = 0) -> some UIView {
        let modifier = ShadowModifier(color: color, radius: radius, x: x, y: y)
        return modifier.body(content: self)
    }
    
    /// Set opacity
    public func opacity(_ opacity: CGFloat) -> some UIView {
        // Implementation for opacity modifier
        return self
    }
    
    /// Set layout priority
    public func layoutPriority(_ priority: Double) -> some UIView {
        // Implementation for layout priority
        return self
    }
}