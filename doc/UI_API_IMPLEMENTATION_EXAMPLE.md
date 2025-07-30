# MiniSwift UI API库集成实现示例

## 第一阶段实现：基础API库集成

### 1. 修改解释器以支持API库调用

#### 1.1 在 `Interpreter.h` 中添加API调用支持

```cpp
// 在 Interpreter.h 中添加
class Interpreter {
private:
    // 现有成员...
    
    // MiniSwift API库调用支持
    Value callMiniSwiftUIFunction(const std::string& functionName, const std::vector<Value>& args);
    std::string serializeValueToJSON(const Value& value);
    Value deserializeValueFromJSON(const std::string& json);
    
    // UI组件句柄管理
    std::map<std::string, std::string> uiWidgetHandles_; // widgetId -> widgetData
    std::string generateWidgetId();
    
public:
    // 现有方法...
    
    // 注册MiniSwift UI函数
    void registerMiniSwiftUIFunctions();
};
```

#### 1.2 在 `Interpreter.cpp` 中实现API调用机制

```cpp
// 添加到 Interpreter.cpp

#include <json/json.h>  // 需要添加JSON库依赖
#include <uuid/uuid.h>  // 用于生成UUID

// C接口声明，用于调用Swift函数
extern "C" {
    char* miniswift_ui_create_text(const char* content);
    char* miniswift_ui_create_button(const char* title, const char* action_id);
    char* miniswift_ui_apply_modifier(const char* widget_id, const char* modifier_type, const char* parameters);
    char* miniswift_ui_render_widget(const char* widget_id);
    void miniswift_ui_cleanup_widget(const char* widget_id);
    int miniswift_ui_run_app(const char* main_widget_id);
}

void Interpreter::registerMiniSwiftUIFunctions() {
    // 注册UI创建函数
    globals->define("__miniswift_createText", Value("<builtin_function:__miniswift_createText>"));
    globals->define("__miniswift_createButton", Value("<builtin_function:__miniswift_createButton>"));
    globals->define("__miniswift_createVStack", Value("<builtin_function:__miniswift_createVStack>"));
    globals->define("__miniswift_createHStack", Value("<builtin_function:__miniswift_createHStack>"));
    
    // 注册修饰符函数
    globals->define("__miniswift_applyModifier", Value("<builtin_function:__miniswift_applyModifier>"));
    
    // 注册应用程序函数
    globals->define("__miniswift_runApp", Value("<builtin_function:__miniswift_runApp>"));
    
    std::cout << "Registered MiniSwift UI API functions" << std::endl;
}

Value Interpreter::callMiniSwiftUIFunction(const std::string& functionName, const std::vector<Value>& args) {
    try {
        if (functionName == "createText") {
            if (args.size() != 1) {
                throw std::runtime_error("createText expects exactly 1 argument");
            }
            
            std::string content = valueToString(args[0]);
            char* result = miniswift_ui_create_text(content.c_str());
            
            if (strncmp(result, "error:", 6) == 0) {
                std::string errorMsg(result + 6);
                free(result);
                throw std::runtime_error("MiniSwift UI Error: " + errorMsg);
            }
            
            std::string widgetId(result);
            free(result);
            
            // 存储widget句柄
            uiWidgetHandles_[widgetId] = "Text(" + content + ")";
            
            return Value("<UIWidget:" + widgetId + ">");
            
        } else if (functionName == "createButton") {
            if (args.size() < 1) {
                throw std::runtime_error("createButton expects at least 1 argument");
            }
            
            std::string title = valueToString(args[0]);
            std::string actionId = generateWidgetId(); // 为action生成ID
            
            char* result = miniswift_ui_create_button(title.c_str(), actionId.c_str());
            
            if (strncmp(result, "error:", 6) == 0) {
                std::string errorMsg(result + 6);
                free(result);
                throw std::runtime_error("MiniSwift UI Error: " + errorMsg);
            }
            
            std::string widgetId(result);
            free(result);
            
            // 存储widget句柄
            uiWidgetHandles_[widgetId] = "Button(" + title + ")";
            
            return Value("<UIWidget:" + widgetId + ">");
            
        } else if (functionName == "applyModifier") {
            if (args.size() < 3) {
                throw std::runtime_error("applyModifier expects at least 3 arguments");
            }
            
            std::string widgetValue = valueToString(args[0]);
            std::string modifierType = valueToString(args[1]);
            
            // 提取widget ID
            std::string widgetId;
            if (widgetValue.find("<UIWidget:") == 0) {
                size_t start = 10; // "<UIWidget:".length()
                size_t end = widgetValue.find(">", start);
                if (end != std::string::npos) {
                    widgetId = widgetValue.substr(start, end - start);
                }
            }
            
            if (widgetId.empty()) {
                throw std::runtime_error("Invalid widget reference");
            }
            
            // 序列化修饰符参数
            Json::Value params(Json::objectValue);
            for (size_t i = 2; i < args.size(); ++i) {
                params["arg" + std::to_string(i-2)] = valueToString(args[i]);
            }
            
            Json::StreamWriterBuilder builder;
            std::string paramStr = Json::writeString(builder, params);
            
            char* result = miniswift_ui_apply_modifier(widgetId.c_str(), modifierType.c_str(), paramStr.c_str());
            
            if (strncmp(result, "error:", 6) == 0) {
                std::string errorMsg(result + 6);
                free(result);
                throw std::runtime_error("MiniSwift UI Error: " + errorMsg);
            }
            
            std::string newWidgetId(result);
            free(result);
            
            // 更新widget句柄
            if (uiWidgetHandles_.find(widgetId) != uiWidgetHandles_.end()) {
                uiWidgetHandles_[newWidgetId] = uiWidgetHandles_[widgetId] + "." + modifierType + "(...)";
                uiWidgetHandles_.erase(widgetId);
            }
            
            return Value("<UIWidget:" + newWidgetId + ">");
            
        } else if (functionName == "runApp") {
            if (args.size() != 1) {
                throw std::runtime_error("runApp expects exactly 1 argument");
            }
            
            std::string widgetValue = valueToString(args[0]);
            std::string widgetId;
            
            // 提取widget ID
            if (widgetValue.find("<UIWidget:") == 0) {
                size_t start = 10;
                size_t end = widgetValue.find(">", start);
                if (end != std::string::npos) {
                    widgetId = widgetValue.substr(start, end - start);
                }
            }
            
            if (widgetId.empty()) {
                throw std::runtime_error("Invalid widget reference for app");
            }
            
            int exitCode = miniswift_ui_run_app(widgetId.c_str());
            return Value(exitCode);
        }
        
        throw std::runtime_error("Unknown MiniSwift UI function: " + functionName);
        
    } catch (const std::exception& e) {
        throw std::runtime_error("MiniSwift UI API Error: " + std::string(e.what()));
    }
}

std::string Interpreter::generateWidgetId() {
    uuid_t uuid;
    uuid_generate(uuid);
    char uuid_str[37];
    uuid_unparse(uuid, uuid_str);
    return std::string(uuid_str);
}

std::string Interpreter::serializeValueToJSON(const Value& value) {
    Json::Value json;
    
    switch (value.type) {
        case ValueType::String:
            json["type"] = "string";
            json["value"] = std::get<std::string>(value.value);
            break;
        case ValueType::Number:
            json["type"] = "number";
            json["value"] = std::get<double>(value.value);
            break;
        case ValueType::Bool:
            json["type"] = "bool";
            json["value"] = std::get<bool>(value.value);
            break;
        default:
            json["type"] = "string";
            json["value"] = valueToString(value);
            break;
    }
    
    Json::StreamWriterBuilder builder;
    return Json::writeString(builder, json);
}

Value Interpreter::deserializeValueFromJSON(const std::string& json) {
    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string errors;
    
    std::istringstream stream(json);
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        throw std::runtime_error("Failed to parse JSON: " + errors);
    }
    
    std::string type = root["type"].asString();
    
    if (type == "string") {
        return Value(root["value"].asString());
    } else if (type == "number") {
        return Value(root["value"].asDouble());
    } else if (type == "bool") {
        return Value(root["value"].asBool());
    }
    
    return Value(root["value"].asString());
}
```

#### 1.3 修改UI组件处理逻辑

```cpp
// 在 visit(const CallExpr& expr) 中修改现有的UI处理逻辑

void Interpreter::visit(const CallExpr& expr) {
    // 现有代码...
    
    // 检查是否是UI组件调用
    if (auto varExpr = dynamic_cast<const VarExpr*>(expr.callee.get())) {
        std::string functionName = varExpr->name.lexeme;
        
        // 使用MiniSwift API库处理UI组件
        if (functionName == "Text") {
            std::vector<Value> args;
            for (const auto& arg : expr.arguments) {
                args.push_back(evaluate(*arg));
            }
            result = callMiniSwiftUIFunction("createText", args);
            return;
        } else if (functionName == "Button") {
            std::vector<Value> args;
            for (const auto& arg : expr.arguments) {
                args.push_back(evaluate(*arg));
            }
            result = callMiniSwiftUIFunction("createButton", args);
            return;
        }
        // 其他UI组件...
    }
    
    // 检查是否是UI方法调用
    if (auto memberExpr = dynamic_cast<const MemberAccessExpr*>(expr.callee.get())) {
        Value object = evaluate(*memberExpr->object);
        std::string memberName = memberExpr->member.lexeme;
        
        // 检查是否是UI widget的方法调用
        if (object.type == ValueType::String) {
            std::string objectStr = std::get<std::string>(object.value);
            if (objectStr.find("<UIWidget:") == 0) {
                // 这是一个UI修饰符调用
                std::vector<Value> args = {object, Value(memberName)};
                for (const auto& arg : expr.arguments) {
                    args.push_back(evaluate(*arg));
                }
                result = callMiniSwiftUIFunction("applyModifier", args);
                return;
            }
        }
    }
    
    // 现有的其他处理逻辑...
}
```

### 2. 扩展MiniSwift API库

#### 2.1 创建UI组件句柄系统

在 `miniswift/Sources/MiniSwift/UI/` 中创建新文件 `UIWidgetHandle.swift`：

```swift
// UIWidgetHandle.swift
import Foundation

/// UI组件句柄，用于在Swift和C++之间传递UI组件引用
public class UIWidgetHandle {
    internal let view: any UIView
    private let id: UUID
    
    internal init(widget: any UIView) {
        self.view = widget
        self.id = UUID()
        UIWidgetRegistry.shared.register(id: id, widget: self)
    }
    
    public func getId() -> String {
        return id.uuidString
    }
    
    public func getView() -> any UIView {
        return view
    }
}

/// UI组件注册表，管理组件生命周期
class UIWidgetRegistry {
    static let shared = UIWidgetRegistry()
    private var widgets: [UUID: UIWidgetHandle] = [:]
    private let queue = DispatchQueue(label: "ui.widget.registry", attributes: .concurrent)
    
    func register(id: UUID, widget: UIWidgetHandle) {
        queue.async(flags: .barrier) {
            self.widgets[id] = widget
        }
    }
    
    func get(id: UUID) -> UIWidgetHandle? {
        return queue.sync {
            return widgets[id]
        }
    }
    
    func get(id: String) -> UIWidgetHandle? {
        guard let uuid = UUID(uuidString: id) else { return nil }
        return get(id: uuid)
    }
    
    func remove(id: UUID) {
        queue.async(flags: .barrier) {
            self.widgets.removeValue(forKey: id)
        }
    }
    
    func remove(id: String) {
        guard let uuid = UUID(uuidString: id) else { return }
        remove(id: uuid)
    }
    
    func cleanup() {
        queue.async(flags: .barrier) {
            self.widgets.removeAll()
        }
    }
}
```

#### 2.2 实现C接口桥接

在 `miniswift/Sources/MiniSwift/` 中创建 `CInterface.swift`：

```swift
// CInterface.swift
import Foundation

// MARK: - C Interface for UI Components

@_cdecl("miniswift_ui_create_text")
public func miniswift_ui_create_text(_ content: UnsafePointer<CChar>) -> UnsafeMutablePointer<CChar> {
    do {
        let swiftContent = String(cString: content)
        let text = Text(swiftContent)
        let handle = UIWidgetHandle(widget: text)
        let widgetId = handle.getId()
        return strdup(widgetId)
    } catch {
        let errorMsg = "error:Failed to create Text: \(error.localizedDescription)"
        return strdup(errorMsg)
    }
}

@_cdecl("miniswift_ui_create_button")
public func miniswift_ui_create_button(_ title: UnsafePointer<CChar>, _ actionId: UnsafePointer<CChar>) -> UnsafeMutablePointer<CChar> {
    do {
        let swiftTitle = String(cString: title)
        let swiftActionId = String(cString: actionId)
        
        // 创建按钮，action暂时为空闭包
        let button = Button(swiftTitle) {
            print("Button clicked: \(swiftActionId)")
            // TODO: 实现action回调机制
        }
        
        let handle = UIWidgetHandle(widget: button)
        let widgetId = handle.getId()
        return strdup(widgetId)
    } catch {
        let errorMsg = "error:Failed to create Button: \(error.localizedDescription)"
        return strdup(errorMsg)
    }
}

@_cdecl("miniswift_ui_apply_modifier")
public func miniswift_ui_apply_modifier(_ widgetId: UnsafePointer<CChar>, _ modifierType: UnsafePointer<CChar>, _ parameters: UnsafePointer<CChar>) -> UnsafeMutablePointer<CChar> {
    do {
        let swiftWidgetId = String(cString: widgetId)
        let swiftModifierType = String(cString: modifierType)
        let swiftParameters = String(cString: parameters)
        
        guard let widget = UIWidgetRegistry.shared.get(id: swiftWidgetId) else {
            return strdup("error:Widget not found")
        }
        
        // 解析参数
        guard let paramData = swiftParameters.data(using: .utf8),
              let paramDict = try JSONSerialization.jsonObject(with: paramData) as? [String: Any] else {
            return strdup("error:Invalid parameters")
        }
        
        // 应用修饰符
        let modifiedWidget = try applyModifierToWidget(widget.getView(), modifierType: swiftModifierType, parameters: paramDict)
        let newHandle = UIWidgetHandle(widget: modifiedWidget)
        
        // 清理旧的widget
        UIWidgetRegistry.shared.remove(id: swiftWidgetId)
        
        return strdup(newHandle.getId())
    } catch {
        let errorMsg = "error:Failed to apply modifier: \(error.localizedDescription)"
        return strdup(errorMsg)
    }
}

@_cdecl("miniswift_ui_render_widget")
public func miniswift_ui_render_widget(_ widgetId: UnsafePointer<CChar>) -> UnsafeMutablePointer<CChar> {
    do {
        let swiftWidgetId = String(cString: widgetId)
        
        guard let widget = UIWidgetRegistry.shared.get(id: swiftWidgetId) else {
            return strdup("error:Widget not found")
        }
        
        let gtkWidget = widget.getView().render()
        // TODO: 返回GTK widget的句柄或描述
        return strdup("rendered:" + swiftWidgetId)
    } catch {
        let errorMsg = "error:Failed to render widget: \(error.localizedDescription)"
        return strdup(errorMsg)
    }
}

@_cdecl("miniswift_ui_run_app")
public func miniswift_ui_run_app(_ mainWidgetId: UnsafePointer<CChar>) -> Int32 {
    do {
        let swiftMainWidgetId = String(cString: mainWidgetId)
        
        guard let mainWidget = UIWidgetRegistry.shared.get(id: swiftMainWidgetId) else {
            print("Error: Main widget not found")
            return -1
        }
        
        // 创建GTK应用程序并运行
        let app = GTK4Application(applicationId: "com.miniswift.app")
        let window = GTK4Widget(type: .window)
        
        // 渲染主widget并添加到窗口
        let mainGTKWidget = mainWidget.getView().render()
        if let gtkWidget = mainGTKWidget as? GTK4Widget {
            window.addChild(gtkWidget)
        }
        
        window.show()
        return Int32(app.run(with: window))
    } catch {
        print("Error running app: \(error.localizedDescription)")
        return -1
    }
}

@_cdecl("miniswift_ui_cleanup_widget")
public func miniswift_ui_cleanup_widget(_ widgetId: UnsafePointer<CChar>) {
    let swiftWidgetId = String(cString: widgetId)
    UIWidgetRegistry.shared.remove(id: swiftWidgetId)
}

// MARK: - Helper Functions

func applyModifierToWidget(_ widget: any UIView, modifierType: String, parameters: [String: Any]) throws -> any UIView {
    switch modifierType {
    case "foregroundColor":
        if let colorName = parameters["arg0"] as? String {
            let color = parseColor(colorName)
            return widget.foregroundColor(color)
        }
        
    case "font":
        if let fontSizeStr = parameters["arg0"] as? String,
           let fontSize = Double(fontSizeStr) {
            let font = Font.system(size: fontSize)
            return widget.font(font)
        }
        
    case "padding":
        if let paddingStr = parameters["arg0"] as? String,
           let padding = Double(paddingStr) {
            return widget.padding(.all, padding)
        }
        
    case "background":
        if let colorName = parameters["arg0"] as? String {
            let color = parseColor(colorName)
            return widget.background(color)
        }
        
    default:
        throw NSError(domain: "UIModifier", code: 1, userInfo: [NSLocalizedDescriptionKey: "Unknown modifier: \(modifierType)"])
    }
    
    return widget
}

func parseColor(_ colorName: String) -> Color {
    switch colorName.lowercased() {
    case "red": return .red
    case "green": return .green
    case "blue": return .blue
    case "black": return .black
    case "white": return .white
    case "gray": return .gray
    case "orange": return .orange
    case "yellow": return .yellow
    case "pink": return .pink
    case "purple": return .purple
    default: return .black
    }
}
```

### 3. 测试示例

创建测试文件 `examples/APILibraryTest.swift`：

```swift
// APILibraryTest.swift
// 测试通过API库创建UI组件

// 创建一个简单的Text组件
let text = Text("Hello from API Library!")
    .font(Font.system(size: 18))
    .foregroundColor(.blue)

// 创建一个Button组件
let button = Button("Click Me") {
    print("Button was clicked!")
}
.padding(10)
.background(.gray)

// 创建一个VStack布局
let content = VStack(spacing: 20) {
    text
    button
}
.padding(20)

// 运行应用程序
UIApplication.shared.setRootView(content)
UIApplication.shared.run()
```

### 4. 构建配置修改

#### 4.1 修改 `CMakeLists.txt`

```cmake
# 添加JSON库依赖
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

# 添加UUID库
find_library(UUID_LIBRARY uuid)

# 链接库
target_link_libraries(miniswift_interpreter 
    ${JSONCPP_LIBRARIES}
    ${UUID_LIBRARY}
    # 其他现有库...
)

# 包含目录
target_include_directories(miniswift_interpreter PRIVATE
    ${JSONCPP_INCLUDE_DIRS}
    # 其他现有目录...
)
```

#### 4.2 修改 `Package.swift`

```swift
// swift-tools-version:5.5
import PackageDescription

let package = Package(
    name: "MiniSwift",
    products: [
        .library(
            name: "MiniSwift",
            type: .dynamic,  // 改为动态库以支持C接口
            targets: ["MiniSwift"]
        ),
    ],
    targets: [
        .target(
            name: "MiniSwift",
            dependencies: [],
            cSettings: [
                .define("SWIFT_PACKAGE")
            ]
        ),
    ]
)
```

### 5. 编译和测试步骤

```bash
# 1. 编译MiniSwift API库
cd miniswift
swift build -c release

# 2. 编译主项目
cd ..
mkdir -p build
cd build
cmake ..
make

# 3. 测试API库集成
./miniswift_interpreter ../examples/APILibraryTest.swift
```

## 总结

这个实现示例展示了如何开始第一阶段的UI API库集成：

1. **解释器修改**: 添加了API库调用机制和UI组件句柄管理
2. **Swift API扩展**: 实现了C接口桥接和组件句柄系统
3. **测试框架**: 提供了完整的测试示例

通过这种方式，UI组件的创建和管理逻辑被转移到了Swift API库中，解释器只负责调用API函数，大大提高了系统的灵活性和可扩展性。

下一步可以继续实现更多的UI组件和修饰符，逐步完善整个UI系统。