#include "UIIntegration.h"
#include "TextWidget.h"
#include "ButtonWidget.h"
#include "VStackWidget.h"
#include "HStackWidget.h"
#include "../interpreter/Value.h"
#include "../interpreter/Environment.h"
#include <iostream>
#include <stdexcept>

namespace MiniSwift {
    namespace UI {
        
        // UIIntegration implementation
        UIIntegration& UIIntegration::getInstance() {
            static UIIntegration instance;
            return instance;
        }
        
        bool UIIntegration::initialize(int argc, char** argv) {
            if (initialized_) {
                return true;
            }
            
            std::cout << "[UIIntegration] Initializing UI system..." << std::endl;
            
            // Select best available backend
            currentBackend_ = selectBestBackend();
            
            // Initialize the selected backend
            switch (currentBackend_) {
                case Backend::GTK4:
                    std::cout << "[UIIntegration] Using GTK4 backend" << std::endl;
                    if (!GTK4::GTK4Application::getInstance().initialize(argc, argv)) {
                        std::cerr << "[UIIntegration] Failed to initialize GTK4 backend, falling back to Mock" << std::endl;
                        currentBackend_ = Backend::Mock;
                        // Initialize Mock backend as fallback
                        if (!UIApplication::getInstance().initialize()) {
                            std::cerr << "[UIIntegration] Failed to initialize Mock backend as fallback" << std::endl;
                            return false;
                        }
                        std::cout << "[UIIntegration] Successfully initialized Mock backend as fallback" << std::endl;
                    }
                    break;
                    
                case Backend::Mock:
                    std::cout << "[UIIntegration] Using Mock backend" << std::endl;
                    if (!UIApplication::getInstance().initialize()) {
                        std::cerr << "[UIIntegration] Failed to initialize Mock backend" << std::endl;
                        return false;
                    }
                    break;
                    
                default:
                    std::cerr << "[UIIntegration] Unknown backend" << std::endl;
                    return false;
            }
            
            initialized_ = true;
            return true;
        }
        
        std::shared_ptr<UIWidget> UIIntegration::createTextFromValue(const MiniSwift::Value& value) {
            std::cout << "[DEBUG] createTextFromValue called" << std::endl;
            if (value.type != miniswift::ValueType::String) {
                throw UIValueError("Text widget requires string value");
            }
            
            std::string text = std::get<std::string>(value.value);
            std::cout << "[DEBUG] Text value: " << text << std::endl;
            std::cout << "[DEBUG] Current backend: " << static_cast<int>(currentBackend_) << std::endl;
            
            std::shared_ptr<UIWidget> widget;
            switch (currentBackend_) {
                case Backend::GTK4:
                    std::cout << "[DEBUG] Calling GTK4::createGTK4Text..." << std::endl;
                    widget = std::static_pointer_cast<UIWidget>(GTK4::createGTK4Text(text));
                    break;
                case Backend::Mock:
                    std::cout << "[DEBUG] Calling createText (Mock)..." << std::endl;
                    widget = createText(text);
                    break;
                default:
                    throw UIBackendError("No backend available for text creation");
            }
            
            // Register the widget in the registry
            if (widget) {
                registerWidget(widget);
            }
            
            return widget;
        }
        
        std::shared_ptr<UIWidget> UIIntegration::createButtonFromValue(const MiniSwift::Value& titleValue, const MiniSwift::Value& actionValue) {
            if (titleValue.type != miniswift::ValueType::String) {
                throw UIValueError("Button title must be a string");
            }
            
            std::string title = std::get<std::string>(titleValue.value);
            
            // Create a callback that handles the action value
            UICallback callback = [actionValue]() {
                std::cout << "[UIIntegration] Button clicked: " << std::get<std::string>(actionValue.value) << std::endl;
                // TODO: Execute the action value if it's a function
            };
            
            std::shared_ptr<UIWidget> widget;
            switch (currentBackend_) {
                case Backend::GTK4:
                    widget = std::static_pointer_cast<UIWidget>(GTK4::createGTK4Button(title, callback));
                    break;
                case Backend::Mock:
                    widget = createButton(title, callback);
                    break;
                default:
                    throw UIBackendError("No backend available for button creation");
            }
            
            // Register the widget in the registry
            if (widget) {
                registerWidget(widget);
            }
            
            return widget;
        }
        
        std::shared_ptr<UIWidget> UIIntegration::createVStackFromValue(const MiniSwift::Value& spacingValue) {
            double spacing = 8.0; // Default spacing
            
            if (spacingValue.type == miniswift::ValueType::Double) {
                spacing = std::get<double>(spacingValue.value);
            }
            
            std::shared_ptr<UIWidget> widget;
            switch (currentBackend_) {
                case Backend::GTK4:
                    widget = std::static_pointer_cast<UIWidget>(GTK4::createGTK4VStack(spacing));
                    break;
                case Backend::Mock:
                    widget = createVStack(spacing);
                    break;
                default:
                    throw UIBackendError("No backend available for VStack creation");
            }
            
            // Register the widget in the registry
            if (widget) {
                registerWidget(widget);
            }
            
            return widget;
        }
        
        std::shared_ptr<UIWidget> UIIntegration::createHStackFromValue(const MiniSwift::Value& spacingValue) {
            double spacing = 8.0; // Default spacing
            
            if (spacingValue.type == miniswift::ValueType::Double) {
                spacing = std::get<double>(spacingValue.value);
            }
            
            std::shared_ptr<UIWidget> widget;
            switch (currentBackend_) {
                case Backend::GTK4:
                    widget = std::static_pointer_cast<UIWidget>(GTK4::createGTK4HStack(spacing));
                    break;
                case Backend::Mock:
                    widget = createHStack(spacing);
                    break;
                default:
                    throw UIBackendError("No backend available for HStack creation");
            }
            
            // Register the widget in the registry
            if (widget) {
                registerWidget(widget);
            }
            
            return widget;
        }
        
        Color UIIntegration::valueToColor(const MiniSwift::Value& value) {
            // TODO: Implement color parsing from various value types
            // For now, return a default color
            return Color{0.0, 0.0, 0.0, 1.0}; // Black
        }
        
        Font UIIntegration::valueToFont(const MiniSwift::Value& value) {
            // TODO: Implement font parsing from value
            return Font{"System", 14.0, false, false};
        }
        
        EdgeInsets UIIntegration::valueToEdgeInsets(const MiniSwift::Value& value) {
            // TODO: Implement edge insets parsing
            return EdgeInsets{8.0, 8.0, 8.0, 8.0};
        }
        
        Size UIIntegration::valueToSize(const MiniSwift::Value& value) {
            // TODO: Implement size parsing
            return Size{100.0, 30.0};
        }
        
        MiniSwift::Value UIIntegration::colorToValue(const Color& color) {
            // TODO: Implement color to value conversion
            return MiniSwift::Value("Color");
        }
        
        MiniSwift::Value UIIntegration::fontToValue(const Font& font) {
            // TODO: Implement font to value conversion
            return MiniSwift::Value(font.family);
        }
        
        MiniSwift::Value UIIntegration::edgeInsetsToValue(const EdgeInsets& insets) {
            // TODO: Implement edge insets to value conversion
            return MiniSwift::Value(insets.top);
        }
        
        MiniSwift::Value UIIntegration::sizeToValue(const Size& size) {
            // TODO: Implement size to value conversion
            return MiniSwift::Value(size.width);
        }
        
        void UIIntegration::addChildToWidget(std::shared_ptr<UIWidget> parent, std::shared_ptr<UIWidget> child) {
            if (!parent || !child) {
                std::cerr << "[UIIntegration] Error: addChildToWidget called with null widget(s)" << std::endl;
                return;
            }
            
            std::cout << "[UIIntegration] Adding child widget to parent" << std::endl;
            parent->addChild(child);
            
            // For GTK4 backend, we need to add to GTK4 container
            if (currentBackend_ == Backend::GTK4) {
                std::cout << "[UIIntegration] Adding child to GTK4 container" << std::endl;
                
                // Try to cast parent to GTK4 stack widgets
                auto gtk4VStack = std::dynamic_pointer_cast<GTK4::GTK4VStackWidget>(parent);
                auto gtk4HStack = std::dynamic_pointer_cast<GTK4::GTK4HStackWidget>(parent);
                
                if (gtk4VStack) {
                    std::cout << "[UIIntegration] Adding child to GTK4 VStack" << std::endl;
                    // Add to GTK4 container (rendering will be handled by parent)
                    gtk4VStack->addGTKChild(child);
                } else if (gtk4HStack) {
                    std::cout << "[UIIntegration] Adding child to GTK4 HStack" << std::endl;
                    // Add to GTK4 container (rendering will be handled by parent)
                    gtk4HStack->addGTKChild(child);
                } else {
                    std::cout << "[UIIntegration] Parent is not a GTK4 stack widget" << std::endl;
                }
            }
            
            std::cout << "[UIIntegration] Child widget added successfully" << std::endl;
        }
        
        std::shared_ptr<UIWidget> UIIntegration::getWidgetFromHandle(const std::string& handle) {
            std::cout << "[UIIntegration] getWidgetFromHandle called with handle: " << handle << std::endl;
            
            auto it = widgetRegistry_.find(handle);
            if (it != widgetRegistry_.end()) {
                std::cout << "[UIIntegration] Found widget for handle: " << handle << std::endl;
                return it->second;
            }
            
            std::cout << "[UIIntegration] Widget not found for handle: " << handle << std::endl;
            return nullptr;
        }
        
        std::string UIIntegration::registerWidget(std::shared_ptr<UIWidget> widget) {
            if (!widget) {
                std::cerr << "[UIIntegration] Error: registerWidget called with null widget" << std::endl;
                return "";
            }
            
            std::string handle = "widget_" + std::to_string(nextWidgetId_++);
            widgetRegistry_[handle] = widget;
            std::cout << "[UIIntegration] Registered widget with handle: " << handle << std::endl;
            return handle;
        }
        
        void UIIntegration::unregisterWidget(const std::string& handle) {
            auto it = widgetRegistry_.find(handle);
            if (it != widgetRegistry_.end()) {
                widgetRegistry_.erase(it);
                std::cout << "[UIIntegration] Unregistered widget with handle: " << handle << std::endl;
            } else {
                std::cout << "[UIIntegration] Widget handle not found for unregistration: " << handle << std::endl;
            }
        }
        
        void UIIntegration::setMainView(std::shared_ptr<UIWidget> view) {
            std::cout << "[UIIntegration] setMainView called" << std::endl;
            mainView_ = view;
            
            switch (currentBackend_) {
                case Backend::GTK4:
                    std::cout << "[UIIntegration] Setting main window content for GTK4" << std::endl;
                    GTK4::GTK4Application::getInstance().setMainWindowContent(view);
                    break;
                case Backend::Mock:
                    std::cout << "[UIIntegration] Setting root widget for Mock" << std::endl;
                    // Render the widget for Mock backend
                    if (view) {
                        view->render();
                    }
                    UIApplication::getInstance().setRootWidget(view);
                    break;
                default:
                    throw UIBackendError("No backend available for setting main view");
            }
            std::cout << "[UIIntegration] setMainView completed" << std::endl;
        }
        
        void UIIntegration::runUIApplication() {
            std::cout << "[UIIntegration] runUIApplication called" << std::endl;
            if (!initialized_) {
                throw UIError("UI system not initialized");
            }
            
            switch (currentBackend_) {
                case Backend::GTK4:
                    std::cout << "[UIIntegration] Running GTK4 application" << std::endl;
                    GTK4::GTK4Application::getInstance().run();
                    break;
                case Backend::Mock:
                    std::cout << "[UIIntegration] Running Mock application" << std::endl;
                    UIApplication::getInstance().run();
                    break;
                default:
                    throw UIBackendError("No backend available for running application");
            }
            std::cout << "[UIIntegration] runUIApplication completed" << std::endl;
        }
        
        void UIIntegration::quitUIApplication() {
            switch (currentBackend_) {
                case Backend::GTK4:
                    GTK4::GTK4Application::getInstance().quit();
                    break;
                case Backend::Mock:
                    UIApplication::getInstance().quit();
                    break;
                default:
                    break;
            }
        }
        
        void UIIntegration::setBackend(Backend backend) {
            if (initialized_) {
                throw UIError("Cannot change backend after initialization");
            }
            currentBackend_ = backend;
        }
        
        UIIntegration::Backend UIIntegration::getCurrentBackend() const {
            return currentBackend_;
        }
        
        bool UIIntegration::isBackendAvailable(Backend backend) const {
            switch (backend) {
                case Backend::GTK4:
                    return GTK4::isGTK4Available();
                case Backend::Mock:
                    return true; // Mock backend is always available
                case Backend::Auto:
                    return true; // Auto selection is always available
                default:
                    return false;
            }
        }
        
        void UIIntegration::updateUI() {
            // TODO: Implement UI update logic
            std::cout << "[UIIntegration] Updating UI..." << std::endl;
        }
        
        void UIIntegration::invalidateLayout() {
            // TODO: Implement layout invalidation
            std::cout << "[UIIntegration] Invalidating layout..." << std::endl;
        }
        
        void UIIntegration::registerEventHandler(const std::string& eventType, std::function<void(const MiniSwift::Value&)> handler) {
            eventHandlers_[eventType] = handler;
        }
        
        void UIIntegration::triggerEvent(const std::string& eventType, const MiniSwift::Value& eventData) {
            auto it = eventHandlers_.find(eventType);
            if (it != eventHandlers_.end()) {
                it->second(eventData);
            }
        }
        
        void UIIntegration::cleanup() {
            if (!initialized_) {
                return;
            }
            
            std::cout << "[UIIntegration] Cleaning up UI system..." << std::endl;
            
            switch (currentBackend_) {
                case Backend::GTK4:
                    GTK4::GTK4Application::getInstance().cleanup();
                    break;
                case Backend::Mock:
                    UIApplication::getInstance().cleanup();
                    break;
                default:
                    break;
            }
            
            mainView_.reset();
            eventHandlers_.clear();
            initialized_ = false;
        }
        
        UIIntegration::Backend UIIntegration::selectBestBackend() {
            if (currentBackend_ != Backend::Auto) {
                return currentBackend_;
            }
            
            // Try GTK4 first
            if (isBackendAvailable(Backend::GTK4)) {
                return Backend::GTK4;
            }
            
            // Fallback to mock
            return Backend::Mock;
        }
        
        // UIInterpreter implementation
        void UIInterpreter::registerUIFunctions(MiniSwift::Environment& env) {
            std::cout << "[UIInterpreter] Registering UI functions..." << std::endl;
            
            // TODO: Register UI functions with the interpreter environment
            // This would involve creating native function objects and adding them to the environment
            
            // Example (pseudo-code):
            // env.define("Text", NativeFunction(createText));
            // env.define("Button", NativeFunction(createButton));
            // env.define("VStack", NativeFunction(createVStack));
            // env.define("HStack", NativeFunction(createHStack));
        }
        
        MiniSwift::Value UIInterpreter::createText(const std::vector<MiniSwift::Value>& args) {
            if (args.empty()) {
                throw UIValueError("Text requires at least one argument");
            }
            
            auto widget = UIIntegration::getInstance().createTextFromValue(args[0]);
            return wrapWidget(widget);
        }
        
        MiniSwift::Value UIInterpreter::createButton(const std::vector<MiniSwift::Value>& args) {
            if (args.size() < 2) {
                throw UIValueError("Button requires title and action arguments");
            }
            
            auto widget = UIIntegration::getInstance().createButtonFromValue(args[0], args[1]);
            return wrapWidget(widget);
        }
        
        MiniSwift::Value UIInterpreter::createVStack(const std::vector<MiniSwift::Value>& args) {
            MiniSwift::Value spacing = args.empty() ? MiniSwift::Value(8.0) : args[0];
            auto widget = UIIntegration::getInstance().createVStackFromValue(spacing);
            return wrapWidget(widget);
        }
        
        MiniSwift::Value UIInterpreter::createHStack(const std::vector<MiniSwift::Value>& args) {
            MiniSwift::Value spacing = args.empty() ? MiniSwift::Value(8.0) : args[0];
            auto widget = UIIntegration::getInstance().createHStackFromValue(spacing);
            return wrapWidget(widget);
        }
        
        MiniSwift::Value UIInterpreter::createColor(const std::vector<MiniSwift::Value>& args) {
            // TODO: Implement color creation
            return MiniSwift::Value("Color");
        }
        
        MiniSwift::Value UIInterpreter::createFont(const std::vector<MiniSwift::Value>& args) {
            // TODO: Implement font creation
            return MiniSwift::Value("Font");
        }
        
        MiniSwift::Value UIInterpreter::applyPadding(const std::vector<MiniSwift::Value>& args) {
            if (args.empty()) {
                return MiniSwift::Value();
            }
            
            // Extract the widget from the first argument
            auto widget = extractWidget(args[0]);
            if (!widget) {
                std::cout << "[UIInterpreter] Warning: applyPadding called with non-widget value" << std::endl;
                return args[0]; // Return original value if not a widget
            }
            
            // TODO: Apply padding to the widget
            // For now, just return the wrapped widget
            return wrapWidget(widget);
        }
        
        MiniSwift::Value UIInterpreter::applyBackground(const std::vector<MiniSwift::Value>& args) {
            if (args.empty()) {
                return MiniSwift::Value();
            }
            
            // Extract the widget from the first argument
            auto widget = extractWidget(args[0]);
            if (!widget) {
                std::cout << "[UIInterpreter] Warning: applyBackground called with non-widget value" << std::endl;
                return args[0]; // Return original value if not a widget
            }
            
            // TODO: Apply background to the widget
            // For now, just return the wrapped widget
            return wrapWidget(widget);
        }
        
        MiniSwift::Value UIInterpreter::applyFrame(const std::vector<MiniSwift::Value>& args) {
            if (args.empty()) {
                return MiniSwift::Value();
            }
            
            // Extract the widget from the first argument
            auto widget = extractWidget(args[0]);
            if (!widget) {
                std::cout << "[UIInterpreter] Warning: applyFrame called with non-widget value" << std::endl;
                return args[0]; // Return original value if not a widget
            }
            
            // TODO: Apply frame to the widget
            // For now, just return the wrapped widget
            return wrapWidget(widget);
        }
        
        MiniSwift::Value UIInterpreter::applyForegroundColor(const std::vector<MiniSwift::Value>& args) {
            if (args.empty()) {
                return MiniSwift::Value();
            }
            
            // Extract the widget from the first argument
            auto widget = extractWidget(args[0]);
            if (!widget) {
                std::cout << "[UIInterpreter] Warning: applyForegroundColor called with non-widget value" << std::endl;
                return args[0]; // Return original value if not a widget
            }
            
            // TODO: Apply foreground color to the widget
            // For now, just return the wrapped widget
            return wrapWidget(widget);
        }
        
        MiniSwift::Value UIInterpreter::applyFont(const std::vector<MiniSwift::Value>& args) {
            if (args.empty()) {
                return MiniSwift::Value();
            }
            
            // Extract the widget from the first argument
            auto widget = extractWidget(args[0]);
            if (!widget) {
                std::cout << "[UIInterpreter] Warning: applyFont called with non-widget value" << std::endl;
                return args[0]; // Return original value if not a widget
            }
            
            // TODO: Apply font to the widget
            // For now, just return the wrapped widget
            return wrapWidget(widget);
        }
        
        MiniSwift::Value UIInterpreter::runApp(const std::vector<MiniSwift::Value>& args) {
            std::cout << "[UIInterpreter] runApp called" << std::endl;
            UIIntegration::getInstance().runUIApplication();
            std::cout << "[UIInterpreter] runApp completed" << std::endl;
            return MiniSwift::Value();
        }
        
        MiniSwift::Value UIInterpreter::setMainView(const std::vector<MiniSwift::Value>& args) {
            std::cout << "[UIInterpreter] setMainView called with " << args.size() << " arguments" << std::endl;
            if (args.empty()) {
                throw UIValueError("setMainView requires a widget argument");
            }
            
            auto widget = extractWidget(args[0]);
            std::cout << "[UIInterpreter] Widget extracted successfully" << std::endl;
            UIIntegration::getInstance().setMainView(widget);
            std::cout << "[UIInterpreter] setMainView completed" << std::endl;
            return MiniSwift::Value();
        }
        
        std::shared_ptr<UIWidget> UIInterpreter::extractWidget(const MiniSwift::Value& value) {
            std::cout << "[UIInterpreter] extractWidget called with value type: " << static_cast<int>(value.type) << std::endl;
            
            if (value.isUIWidget()) {
                auto widgetValue = std::get<std::shared_ptr<MiniSwift::UI::UIWidgetValue>>(value.value);
                if (widgetValue) {
                    return widgetValue->getWidget();
                }
            }
            
            std::cout << "[UIInterpreter] Warning: extractWidget called with non-UIWidget value, returning nullptr" << std::endl;
            return nullptr;
        }
        
        MiniSwift::Value UIInterpreter::wrapWidget(std::shared_ptr<UIWidget> widget) {
            if (!widget) {
                std::cout << "[UIInterpreter] Warning: wrapWidget called with null widget" << std::endl;
                return MiniSwift::Value();
            }
            
            auto widgetValue = std::make_shared<MiniSwift::UI::UIWidgetValue>(widget);
            return MiniSwift::Value(widgetValue);
        }
        
        // UIWidgetValue implementation
        std::string UIWidgetValue::toString() const {
            return "UIWidget";
        }
        
        bool UIWidgetValue::equals(const UIWidgetValue& other) const {
            return widget_ == other.widget_;
        }
        
        // UIConstants implementation
        namespace UIConstants {
            const Color BLACK = {0.0, 0.0, 0.0, 1.0};
            const Color WHITE = {1.0, 1.0, 1.0, 1.0};
            const Color RED = {1.0, 0.0, 0.0, 1.0};
            const Color GREEN = {0.0, 1.0, 0.0, 1.0};
            const Color BLUE = {0.0, 0.0, 1.0, 1.0};
            const Color CLEAR = {0.0, 0.0, 0.0, 0.0};
            const Color GRAY = {0.6, 0.6, 0.6, 1.0};


            const Font SYSTEM_FONT = {"System", 14.0, false, false};
            const Font TITLE_FONT = {"System", 18.0, true, false};
            const Font CAPTION_FONT = {"System", 12.0, false, false};
            
            const double DEFAULT_SPACING = 8.0;
            const EdgeInsets DEFAULT_PADDING = {8.0, 8.0, 8.0, 8.0};
            const Size DEFAULT_BUTTON_SIZE = {100.0, 30.0};
        }
        
    } // namespace UI
} // namespace MiniSwift