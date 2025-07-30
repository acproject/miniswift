// MiniSwift Library - IO, Network and UI APIs

import Foundation

// Global error tracking
private var lastError: String?

// C string helper function
private func createCString(_ string: String) -> UnsafePointer<CChar>? {
    return string.withCString { cString in
        let length = strlen(cString) + 1
        let buffer = UnsafeMutablePointer<CChar>.allocate(capacity: length)
        strcpy(buffer, cString)
        return UnsafePointer(buffer)
    }
}

// MARK: - UI Module
// UI components and types are available when importing MiniSwift
// All UI types are defined in separate files under UI/ directory

// MARK: - IO and Network APIs

/// Write content to a file
/// - Parameters:
///   - filename: The name of the file to write to
///   - content: The content to write to the file
/// - Returns: True if successful, false otherwise
public func writeFile(_ filename: String, _ content: String) -> Bool {
    // This will be handled by the interpreter's builtin function
    return __builtin_writeFile(filename, content)
}

/// Read content from a file
/// - Parameter filename: The name of the file to read from
/// - Returns: The content of the file as a string
public func readFile(_ filename: String) -> String {
    // This will be handled by the interpreter's builtin function
    return __builtin_readFile(filename)
}

/// Check if a file exists
/// - Parameter filename: The name of the file to check
/// - Returns: True if the file exists, false otherwise
public func fileExists(_ filename: String) -> Bool {
    // This will be handled by the interpreter's builtin function
    return __builtin_fileExists(filename)
}

/// Make an HTTP GET request
/// - Parameter url: The URL to make the request to
/// - Returns: The response content as a string
public func httpGet(_ url: String) -> String {
    // This will be handled by the interpreter's builtin function
    return __builtin_httpGet(url)
}

/// Make an HTTP POST request
/// - Parameters:
///   - url: The URL to make the request to
///   - data: The data to send in the request body
/// - Returns: The response content as a string
public func httpPost(_ url: String, _ data: String) -> String {
    // This will be handled by the interpreter's builtin function
    return __builtin_httpPost(url, data)
}

// MARK: - UI Component Creation Functions

/// UI组件句柄，用于在Swift和C++之间传递UI组件引用
public class UIWidgetHandle {
    internal let view: any UIView
    private let id: String
    
    internal init(widget: any UIView) {
        self.view = widget
        self.id = UUID().uuidString
        UIWidgetRegistry.shared.register(id: id, widget: self)
    }
    
    public func getId() -> String {
        return id
    }
}

/// UI组件注册表，管理组件生命周期
class UIWidgetRegistry {
    static let shared = UIWidgetRegistry()
    private var widgets: [String: UIWidgetHandle] = [:]
    
    func register(id: String, widget: UIWidgetHandle) {
        widgets[id] = widget
    }
    
    func get(id: String) -> UIWidgetHandle? {
        return widgets[id]
    }
    
    func remove(id: String) {
        widgets.removeValue(forKey: id)
    }
}

/// Create a Text component
public func createText(_ content: String) -> UIWidgetHandle {
    let text = Text(content)
    return UIWidgetHandle(widget: text)
}

/// Create a Button component
public func createButton(_ title: String, action: @escaping () -> Void = {}) -> UIWidgetHandle {
    let button = Button(title, action: action)
    return UIWidgetHandle(widget: button)
}

/// Create a VStack component
public func createVStack(spacing: Double = 8, @ViewBuilder content: () -> any UIView) -> UIWidgetHandle {
    let vstack = VStack(spacing: spacing, content: content)
    return UIWidgetHandle(widget: vstack)
}

/// Create an HStack component
public func createHStack(spacing: Double = 8, @ViewBuilder content: () -> any UIView) -> UIWidgetHandle {
    let hstack = HStack(spacing: spacing, content: content)
    return UIWidgetHandle(widget: hstack)
}

/// Apply a modifier to a UI component
public func applyModifier(_ widget: UIWidgetHandle, _ modifierType: String, _ parameters: [String: Any]) -> UIWidgetHandle {
    // 根据modifierType应用相应的修饰符
    switch modifierType {
    case "foregroundColor":
        if let colorName = parameters["color"] as? String {
            let color = parseColor(colorName)
            return UIWidgetHandle(widget: widget.view.foregroundColor(color))
        }
    case "font":
        if let fontSize = parameters["size"] as? Double {
            let font = Font.system(size: fontSize)
            return UIWidgetHandle(widget: widget.view.font(font))
        }
    case "padding":
        if let padding = parameters["padding"] as? Double {
            return UIWidgetHandle(widget: widget.view.padding(padding))
        }
    case "background":
        if let colorName = parameters["color"] as? String {
            let color = parseColor(colorName)
            return UIWidgetHandle(widget: widget.view.background(color))
        }
    default:
        break
    }
    return widget
}

/// Parse color from string
private func parseColor(_ colorName: String) -> Color {
    switch colorName.lowercased() {
    case "red": return .red
    case "blue": return .blue
    case "green": return .green
    case "black": return .black
    case "white": return .white
    case "gray": return .gray
    case "yellow": return .yellow
    case "orange": return .orange
    case "purple": return .purple
    case "pink": return .pink
    case "clear": return .clear
    default: return .black
    }
}

// MARK: - C Interface Bridge

/// C接口：创建Text组件
@_cdecl("miniswift_ui_create_text")
public func miniswift_ui_create_text(_ content: UnsafePointer<CChar>, 
                                   _ handlePtr: UnsafeMutablePointer<UnsafePointer<CChar>?>) -> Bool {
    let swiftContent = String(cString: content)
    let widget = createText(swiftContent)
    let handle = widget.getId()
    
    handlePtr.pointee = createCString(handle)
    return true
}

/// C接口：创建Button组件
@_cdecl("miniswift_ui_create_button")
public func miniswift_ui_create_button(_ title: UnsafePointer<CChar>,
                                     _ handlePtr: UnsafeMutablePointer<UnsafePointer<CChar>?>) -> Bool {
    let swiftTitle = String(cString: title)
    let widget = createButton(swiftTitle)
    let handle = widget.getId()
    
    handlePtr.pointee = createCString(handle)
    return true
}

/// C接口：应用修饰符
@_cdecl("miniswift_ui_apply_modifier")
public func miniswift_ui_apply_modifier(_ widgetHandle: UnsafePointer<CChar>,
                                      _ modifierType: UnsafePointer<CChar>,
                                      _ params: UnsafePointer<CChar>) -> UnsafeMutablePointer<CChar>? {
    let swiftWidgetHandle = String(cString: widgetHandle)
    let swiftModifierType = String(cString: modifierType)
    let swiftParams = String(cString: params)
    
    guard let widget = UIWidgetRegistry.shared.get(id: swiftWidgetHandle) else {
        return createCString("error:widget_not_found")
    }
    
    // 解析JSON参数 - 简化版本，直接使用字符串参数
    let parameters: [String: Any] = [:] // 简化处理，实际应用中可以解析JSON
    
    let modifiedWidget = applyModifier(widget, swiftModifierType, parameters)
    return createCString(modifiedWidget.getId())
}

/// C接口：获取最后的错误信息
@_cdecl("miniswift_ui_get_last_error")
public func miniswift_ui_get_last_error() -> UnsafePointer<CChar>? {
    guard let error = lastError else { return nil }
    return createCString(error)
}

/// C接口：清理组件
@_cdecl("miniswift_ui_cleanup_widget")
public func miniswift_ui_cleanup_widget(_ widgetHandle: UnsafePointer<CChar>) {
    let swiftWidgetHandle = String(cString: widgetHandle)
    UIWidgetRegistry.shared.remove(id: swiftWidgetHandle)
}