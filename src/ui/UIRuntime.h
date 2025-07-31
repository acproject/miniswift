#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <any>

// Forward declarations
class UIView;
class UIWidget;
class UIApplication;

namespace MiniSwift {
    namespace UI {
        
        // UI Runtime types
        using UICallback = std::function<void()>;
        using UIPropertyMap = std::map<std::string, std::any>;
        
        // Color representation
        struct Color {
            double red, green, blue, alpha;
            
            Color(double r = 0.0, double g = 0.0, double b = 0.0, double a = 1.0)
                : red(r), green(g), blue(b), alpha(a) {}
            
            // Predefined colors
            static Color black() { return Color(0, 0, 0, 1); }
            static Color white() { return Color(1, 1, 1, 1); }
            static Color redColor() { return Color(1, 0, 0, 1); }
            static Color greenColor() { return Color(0, 1, 0, 1); }
            static Color blueColor() { return Color(0, 0, 1, 1); }
            static Color clear() { return Color(0, 0, 0, 0); }
        };
        
        // Size and position
        struct Size {
            double width, height;
            Size(double w = 0, double h = 0) : width(w), height(h) {}
        };
        
        struct Point {
            double x, y;
            Point(double x = 0, double y = 0) : x(x), y(y) {}
        };
        
        struct Rect {
            Point origin;
            Size size;
            Rect(Point o = Point(), Size s = Size()) : origin(o), size(s) {}
            Rect(double x, double y, double w, double h) : origin(x, y), size(w, h) {}
        };
        
        // Edge insets for padding/margins
        struct EdgeInsets {
            double top, left, bottom, right;
            EdgeInsets(double t = 0, double l = 0, double b = 0, double r = 0)
                : top(t), left(l), bottom(b), right(r) {}
            EdgeInsets(double all) : top(all), left(all), bottom(all), right(all) {}
        };
        
        // Alignment types
        enum class HorizontalAlignment {
            Leading, Center, Trailing
        };
        
        enum class VerticalAlignment {
            Top, Center, Bottom
        };
        
        struct Alignment {
            HorizontalAlignment horizontal;
            VerticalAlignment vertical;
            
            Alignment(HorizontalAlignment h = HorizontalAlignment::Center,
                     VerticalAlignment v = VerticalAlignment::Center)
                : horizontal(h), vertical(v) {}
            
            static Alignment topLeading() { return Alignment(HorizontalAlignment::Leading, VerticalAlignment::Top); }
            static Alignment center() { return Alignment(HorizontalAlignment::Center, VerticalAlignment::Center); }
            static Alignment bottomTrailing() { return Alignment(HorizontalAlignment::Trailing, VerticalAlignment::Bottom); }
        };
        
        // Font representation
        struct Font {
            std::string family;
            double size;
            bool bold;
            bool italic;
            
            Font(const std::string& f = "System", double s = 14.0, bool b = false, bool i = false)
                : family(f), size(s), bold(b), italic(i) {}
            
            static Font system(double size = 14.0) { return Font("System", size); }
            static Font title() { return Font("System", 28.0, true); }
            static Font headline() { return Font("System", 17.0, true); }
            static Font body() { return Font("System", 17.0); }
            static Font caption() { return Font("System", 12.0); }
        };
        
        // UI Widget types
        enum class WidgetType {
            Text,
            Button,
            Image,
            Rectangle,
            VStack,
            HStack,
            ZStack,
            Grid,
            Spacer,
            Divider
        };
        
        // Base UI Widget class
        class UIWidget {
        public:
            UIWidget(WidgetType type) : type_(type) {}
            virtual ~UIWidget() = default;
            
            WidgetType getType() const { return type_; }
            
            // Properties
            void setFrame(const Rect& frame) { frame_ = frame; }
            Rect getFrame() const { return frame_; }
            
            void setPadding(const EdgeInsets& padding) { padding_ = padding; }
            EdgeInsets getPadding() const { return padding_; }
            
            void setBackgroundColor(const Color& color) { backgroundColor_ = color; }
            Color getBackgroundColor() const { return backgroundColor_; }
            
            void setCornerRadius(double radius) { cornerRadius_ = radius; }
            double getCornerRadius() const { return cornerRadius_; }
            
            // Child management
            void addChild(std::shared_ptr<UIWidget> child) { children_.push_back(child); }
            const std::vector<std::shared_ptr<UIWidget>>& getChildren() const { return children_; }
            
            // Rendering (to be implemented by platform-specific backends)
            virtual void render() = 0;
            virtual void layout() = 0;
            
        protected:
            WidgetType type_;
            Rect frame_;
            EdgeInsets padding_;
            Color backgroundColor_ = Color::clear();
            double cornerRadius_ = 0.0;
            std::vector<std::shared_ptr<UIWidget>> children_;
        };
        
        // Forward declarations for UI widgets
        class TextWidget;
        class ButtonWidget;
        class VStackWidget;
        class HStackWidget;
        
        // UI Application manager
        class UIApplication {
        public:
            static UIApplication& getInstance();
            
            void setRootWidget(std::shared_ptr<UIWidget> root);
            std::shared_ptr<UIWidget> getRootWidget() const { return rootWidget_; }
            
            void run();
            void quit();
            
            // Platform-specific initialization
            bool initialize();
            void cleanup();
            
        private:
            UIApplication() = default;
            std::shared_ptr<UIWidget> rootWidget_;
            bool running_ = false;
        };
        
        // Factory functions for creating widgets (declared in individual widget files)
        // std::shared_ptr<TextWidget> createText(const std::string& text);
        // std::shared_ptr<ButtonWidget> createButton(const std::string& title, UICallback callback = nullptr);
        // std::shared_ptr<VStackWidget> createVStack(double spacing = 0);
        // std::shared_ptr<HStackWidget> createHStack(double spacing = 0);
        
    } // namespace UI
} // namespace MiniSwift