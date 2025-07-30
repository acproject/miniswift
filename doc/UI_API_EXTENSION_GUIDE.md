# MiniSwift UI库扩展指南

## 当前架构分析

### 1. 现有UI库结构

当前的MiniSwift项目包含两个UI实现层次：

#### 底层UI实现 (`src/ui/`)
- **UIIntegration.h/cpp**: 核心UI集成层，提供Swift值与C++ UI运行时的桥接
- **UIRuntime.h/cpp**: UI运行时系统
- **GTK4Backend.h/cpp**: GTK4后端实现
- **UITest.cpp**: UI测试代码

#### 解释器UI处理 (`src/interpreter/Interpreter.cpp`)
- 内置UI组件处理：Text, Button, VStack, HStack, Color
- UI方法链调用：foregroundColor, font, padding, background
- UIApplication.shared 支持

#### 高级API库 (`miniswift/Sources/MiniSwift/UI/`)
- **UIView.swift**: 基础UI协议和修饰符系统
- **Components.swift**: UI组件（Text, Button, Image, Rectangle, Spacer等）
- **Layout.swift**: 布局容器（VStack, HStack, ZStack, Grid）
- **Modifiers.swift**: 视图修饰符系统
- **Style.swift**: 样式系统（Color, Font, 对齐等）
- **GTK4Bridge.swift**: GTK4桥接层
- **Geometry.swift**: 几何类型定义

### 2. 当前问题分析

#### 灵活性限制
1. **硬编码组件**: 解释器中UI组件处理是硬编码的
2. **有限的修饰符**: 只支持基础的UI修饰符
3. **缺乏扩展性**: 添加新UI组件需要修改底层C++代码
4. **协议支持不足**: 不支持SwiftUI风格的协议和计算属性

#### 架构分离问题
1. **双重实现**: 底层C++实现和Swift API库实现分离
2. **同步困难**: 两层实现需要手动保持同步
3. **功能差异**: Swift API库功能更丰富，但解释器支持有限

## 扩展方案设计

### 方案1: 通过MiniSwift API库扩展（推荐）

#### 核心思路
将UI组件的创建和处理逻辑从解释器转移到Swift API库，解释器只负责调用API库的函数。

#### 实现步骤

##### 1. 扩展解释器的API库支持

在 `Interpreter.cpp` 中添加对MiniSwift库函数的动态调用支持：

```cpp
// 添加到 registerBuiltinFunctions()
void Interpreter::registerBuiltinFunctions() {
    // 现有代码...
    
    // 注册MiniSwift UI库函数
    globals->define("__miniswift_createText", Value("<builtin_function:__miniswift_createText>"));
    globals->define("__miniswift_createButton", Value("<builtin_function:__miniswift_createButton>"));
    globals->define("__miniswift_createVStack", Value("<builtin_function:__miniswift_createVStack>"));
    globals->define("__miniswift_createHStack", Value("<builtin_function:__miniswift_createHStack>"));
    globals->define("__miniswift_applyModifier", Value("<builtin_function:__miniswift_applyModifier>"));
}
```

##### 2. 修改UI组件处理逻辑

将现有的硬编码UI处理替换为API库调用：

```cpp
// 在 visit(const CallExpr& expr) 中
if (functionName == "Text") {
    // 调用MiniSwift库的Text创建函数
    std::vector<Value> args;
    for (const auto& arg : expr.arguments) {
        args.push_back(evaluate(*arg));
    }
    result = callMiniSwiftFunction("createText", args);
    return;
}
```

##### 3. 实现MiniSwift库函数调用机制

```cpp
Value Interpreter::callMiniSwiftFunction(const std::string& functionName, const std::vector<Value>& args) {
    // 1. 将参数序列化为JSON或其他格式
    // 2. 调用对应的Swift函数
    // 3. 解析返回值
    // 4. 返回Value对象
}
```

##### 4. 扩展MiniSwift API库

在 `MiniSwift.swift` 中添加UI创建函数：

```swift
// MARK: - UI Creation Functions

/// Create a Text component
public func createText(_ content: String) -> UIWidgetHandle {
    let text = Text(content)
    return UIWidgetHandle(widget: text)
}

/// Create a Button component
public func createButton(_ title: String, action: @escaping () -> Void) -> UIWidgetHandle {
    let button = Button(title, action: action)
    return UIWidgetHandle(widget: button)
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
    // 更多修饰符...
    }
    return widget
}
```

##### 5. 创建UI组件句柄系统

```swift
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
}

/// UI组件注册表，管理组件生命周期
class UIWidgetRegistry {
    static let shared = UIWidgetRegistry()
    private var widgets: [UUID: UIWidgetHandle] = [:]
    
    func register(id: UUID, widget: UIWidgetHandle) {
        widgets[id] = widget
    }
    
    func get(id: UUID) -> UIWidgetHandle? {
        return widgets[id]
    }
    
    func remove(id: UUID) {
        widgets.removeValue(forKey: id)
    }
}
```

### 方案2: 插件化UI组件系统

#### 核心思路
创建一个插件化的UI组件系统，允许通过配置文件或动态加载来扩展UI组件。

#### 实现要点

1. **组件注册机制**
```cpp
class UIComponentRegistry {
public:
    using ComponentFactory = std::function<std::shared_ptr<UIWidget>(const std::vector<Value>&)>;
    
    void registerComponent(const std::string& name, ComponentFactory factory);
    std::shared_ptr<UIWidget> createComponent(const std::string& name, const std::vector<Value>& args);
    
private:
    std::map<std::string, ComponentFactory> components_;
};
```

2. **修饰符注册机制**
```cpp
class UIModifierRegistry {
public:
    using ModifierApplier = std::function<std::shared_ptr<UIWidget>(std::shared_ptr<UIWidget>, const std::vector<Value>&)>;
    
    void registerModifier(const std::string& name, ModifierApplier applier);
    std::shared_ptr<UIWidget> applyModifier(const std::string& name, std::shared_ptr<UIWidget> widget, const std::vector<Value>& args);
    
private:
    std::map<std::string, ModifierApplier> modifiers_;
};
```

### 方案3: DSL (Domain Specific Language) 方法

#### 核心思路
创建一个专门的UI DSL，通过解析DSL来生成UI组件。

#### 实现要点

1. **DSL语法设计**
```swift
// DSL示例
ui {
    VStack(spacing: 10) {
        Text("Hello World")
            .font(.title)
            .foregroundColor(.blue)
        
        Button("Click Me") {
            print("Button clicked")
        }
        .padding()
        .background(.gray)
    }
}
```

2. **DSL解析器**
```cpp
class UIDSLParser {
public:
    std::shared_ptr<UIWidget> parseUIDescription(const std::string& dsl);
    
private:
    std::shared_ptr<UIWidget> parseComponent(const ASTNode& node);
    void applyModifiers(std::shared_ptr<UIWidget> widget, const std::vector<ASTNode>& modifiers);
};
```

## 推荐实施路径

### 阶段1: 基础API库集成（1-2周）

1. **扩展解释器API调用能力**
   - 实现MiniSwift库函数调用机制
   - 添加参数序列化/反序列化
   - 实现返回值处理

2. **创建UI组件句柄系统**
   - 实现UIWidgetHandle类
   - 创建组件注册表
   - 实现生命周期管理

3. **基础组件迁移**
   - 将Text组件迁移到API库
   - 将Button组件迁移到API库
   - 测试基础功能

### 阶段2: 修饰符系统集成（2-3周）

1. **修饰符调用机制**
   - 实现修饰符链式调用
   - 添加参数传递支持
   - 实现修饰符组合

2. **常用修饰符迁移**
   - foregroundColor
   - font
   - padding
   - background
   - frame

3. **布局组件支持**
   - VStack/HStack迁移
   - 布局参数处理
   - 子组件管理

### 阶段3: 高级功能扩展（3-4周）

1. **复杂组件支持**
   - Image组件
   - Grid布局
   - ZStack层叠
   - 自定义组件

2. **事件处理系统**
   - 按钮点击事件
   - 手势识别
   - 状态管理

3. **样式系统完善**
   - 主题支持
   - 动画系统
   - 响应式布局

### 阶段4: 性能优化和工具支持（2-3周）

1. **性能优化**
   - 组件缓存机制
   - 渲染优化
   - 内存管理优化

2. **开发工具**
   - UI预览工具
   - 调试支持
   - 热重载功能

## 技术实现细节

### 1. 跨语言调用机制

#### C++调用Swift函数
```cpp
// 使用C接口桥接
extern "C" {
    char* miniswift_create_text(const char* content);
    char* miniswift_apply_modifier(const char* widget_id, const char* modifier_type, const char* parameters);
    void miniswift_cleanup_widget(const char* widget_id);
}

Value Interpreter::callMiniSwiftFunction(const std::string& functionName, const std::vector<Value>& args) {
    if (functionName == "createText") {
        std::string content = valueToString(args[0]);
        char* result = miniswift_create_text(content.c_str());
        std::string widgetId(result);
        free(result);
        return Value("<UIWidget:" + widgetId + ">");
    }
    // 其他函数处理...
}
```

#### Swift实现C接口
```swift
@_cdecl("miniswift_create_text")
public func miniswift_create_text(_ content: UnsafePointer<CChar>) -> UnsafeMutablePointer<CChar> {
    let swiftContent = String(cString: content)
    let widget = createText(swiftContent)
    let widgetId = widget.getId()
    return strdup(widgetId)
}

@_cdecl("miniswift_apply_modifier")
public func miniswift_apply_modifier(_ widgetId: UnsafePointer<CChar>, _ modifierType: UnsafePointer<CChar>, _ parameters: UnsafePointer<CChar>) -> UnsafeMutablePointer<CChar> {
    let swiftWidgetId = String(cString: widgetId)
    let swiftModifierType = String(cString: modifierType)
    let swiftParameters = String(cString: parameters)
    
    guard let widget = UIWidgetRegistry.shared.get(id: UUID(uuidString: swiftWidgetId)!) else {
        return strdup("error:widget_not_found")
    }
    
    // 解析参数并应用修饰符
    let modifiedWidget = applyModifier(widget, swiftModifierType, parseParameters(swiftParameters))
    return strdup(modifiedWidget.getId())
}
```

### 2. 参数序列化格式

使用JSON格式进行参数传递：

```json
{
    "type": "modifier",
    "name": "foregroundColor",
    "parameters": {
        "color": "blue"
    }
}
```

### 3. 错误处理机制

```cpp
class UIAPIError : public std::runtime_error {
public:
    UIAPIError(const std::string& message) : std::runtime_error("UI API Error: " + message) {}
};

Value Interpreter::callMiniSwiftFunction(const std::string& functionName, const std::vector<Value>& args) {
    try {
        // 调用Swift函数
        char* result = call_swift_function(functionName.c_str(), serialize_args(args).c_str());
        
        if (strncmp(result, "error:", 6) == 0) {
            std::string errorMsg(result + 6);
            free(result);
            throw UIAPIError(errorMsg);
        }
        
        std::string resultStr(result);
        free(result);
        return Value(resultStr);
    } catch (const std::exception& e) {
        throw UIAPIError("Failed to call Swift function '" + functionName + "': " + e.what());
    }
}
```

## 总结

通过MiniSwift API库扩展UI功能是一个可行且优雅的解决方案。这种方法的主要优势包括：

1. **分离关注点**: UI逻辑在Swift层实现，解释器专注于语言特性
2. **易于扩展**: 新增UI组件只需修改Swift代码
3. **类型安全**: 利用Swift的类型系统确保UI组件的正确性
4. **维护性好**: 减少C++代码的复杂性，提高代码可读性
5. **功能丰富**: 可以充分利用Swift API库的完整功能

建议按照推荐的阶段性实施路径，逐步将UI功能从解释器迁移到API库，最终实现一个灵活、可扩展的UI系统。