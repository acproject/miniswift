#include "UIIntegration.h"
#include "TextWidget.h"
#include "ButtonWidget.h"
#include "VStackWidget.h"
#include "HStackWidget.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>

#if defined(MINISWIFT_UI_USE_DUOROU)
#include <duorou/ui/runtime.hpp>
#endif

namespace MiniSwift {
    namespace UI {

#if defined(MINISWIFT_UI_USE_DUOROU)
        namespace {
            static std::int64_t color_to_argb32(const Color& c) {
                auto clamp01 = [](double v) -> double {
                    if (v < 0.0) return 0.0;
                    if (v > 1.0) return 1.0;
                    return v;
                };
                const std::uint32_t a = static_cast<std::uint32_t>(clamp01(c.alpha) * 255.0 + 0.5);
                const std::uint32_t r = static_cast<std::uint32_t>(clamp01(c.red) * 255.0 + 0.5);
                const std::uint32_t g = static_cast<std::uint32_t>(clamp01(c.green) * 255.0 + 0.5);
                const std::uint32_t b = static_cast<std::uint32_t>(clamp01(c.blue) * 255.0 + 0.5);
                return static_cast<std::int64_t>((a << 24) | (r << 16) | (g << 8) | b);
            }

            static double edge_insets_to_uniform_padding(const EdgeInsets& e) {
                if (e.top == e.left && e.top == e.bottom && e.top == e.right) {
                    return e.top;
                }
                return std::max(std::max(e.top, e.bottom), std::max(e.left, e.right));
            }

            static ::duorou::ui::ViewNode to_duorou_tree(const std::shared_ptr<UIWidget>& w) {
                using namespace ::duorou::ui;
                if (!w) {
                    return ViewNode{};
                }

                ViewBuilder b = [&]() -> ViewBuilder {
                    switch (w->getType()) {
                    case WidgetType::Text: {
                        auto tw = std::dynamic_pointer_cast<TextWidget>(w);
                        ViewBuilder out = view("Text");
                        out.prop("value", tw ? tw->getText() : std::string{});
                        if (tw) {
                            out.prop("font_size", tw->getFont().size);
                            out.prop("color", color_to_argb32(tw->getTextColor()));
                        }
                        return out;
                    }
                    case WidgetType::Button: {
                        auto bw = std::dynamic_pointer_cast<ButtonWidget>(w);
                        ViewBuilder out = view("Button");
                        out.prop("title", bw ? bw->getTitle() : std::string{});
                        if (bw) {
                            out.event("pointer_up", on_pointer_up([bw]() mutable { bw->onClick(); }));
                        }
                        return out;
                    }
                    case WidgetType::VStack: {
                        auto vw = std::dynamic_pointer_cast<VStackWidget>(w);
                        ViewBuilder out = view("Column");
                        if (vw) {
                            out.prop("spacing", vw->getSpacing());
                        }
                        return out;
                    }
                    case WidgetType::HStack: {
                        auto hw = std::dynamic_pointer_cast<HStackWidget>(w);
                        ViewBuilder out = view("Row");
                        if (hw) {
                            out.prop("spacing", hw->getSpacing());
                        }
                        return out;
                    }
                    default:
                        return view("Box");
                    }
                }();

                const auto pad = edge_insets_to_uniform_padding(w->getPadding());
                if (pad > 0.0) {
                    b.prop("padding", pad);
                }

                const auto bg = w->getBackgroundColor();
                if (bg.alpha > 0.0) {
                    b.prop("bg", color_to_argb32(bg));
                }

                const auto f = w->getFrame();
                if (f.size.width > 0.0) {
                    b.prop("width", f.size.width);
                }
                if (f.size.height > 0.0) {
                    b.prop("height", f.size.height);
                }

                const auto& kids = w->getChildren();
                if (!kids.empty()) {
                    b.children([&](auto& c) {
                        for (const auto& ch : kids) {
                            c.add(to_duorou_tree(ch));
                        }
                    });
                }

                return std::move(b).build();
            }
        }
#endif
        
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
            (void)argc;
            (void)argv;

            currentBackend_ = selectBestBackend();
            if (currentBackend_ == Backend::Duorou) {
                std::cout << "[UIIntegration] Using duorou backend" << std::endl;
            } else {
                std::cout << "[UIIntegration] Using mock backend" << std::endl;
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
            widget = createText(text);
            
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
            widget = createButton(title, callback);
            
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
            widget = createVStack(spacing);
            
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
            widget = createHStack(spacing);
            
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
            std::cout << "[UIIntegration] setMainView completed" << std::endl;
        }
        
        void UIIntegration::runUIApplication() {
            std::cout << "[UIIntegration] runUIApplication called" << std::endl;
            if (!initialized_) {
                throw UIError("UI system not initialized");
            }
            std::cout << "[UIIntegration] runUIApplication completed" << std::endl;
        }
        
        void UIIntegration::quitUIApplication() {
            return;
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
                case Backend::Duorou:
#if defined(MINISWIFT_UI_USE_DUOROU)
                    return true;
#else
                    return false;
#endif
                case Backend::Mock:
                    return true;
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
                case Backend::Duorou:
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
#if defined(MINISWIFT_UI_USE_DUOROU)
            return Backend::Duorou;
#else
            return Backend::Mock;
#endif
        }

#if defined(MINISWIFT_UI_USE_DUOROU)
        ::duorou::ui::ViewNode UIIntegration::buildDuorouTree(std::shared_ptr<UIWidget> root) const {
            return to_duorou_tree(root);
        }

        ::duorou::ui::ViewNode UIIntegration::buildDuorouMainView() const {
            return to_duorou_tree(mainView_);
        }
#endif
        
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
