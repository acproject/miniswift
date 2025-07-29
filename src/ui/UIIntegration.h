#pragma once

#include "UIRuntime.h"
#include "GTK4Backend.h"
#include "../interpreter/Value.h"
#include "../interpreter/Environment.h"
#include <memory>
#include <string>
#include <functional>
#include <map>

// Forward declarations
namespace MiniSwift {
    class Value;
    class Environment;
}

namespace MiniSwift {
    namespace UI {
        
        // Forward declarations
        class UIInterpreter;
        
        /**
         * UI Integration with MiniSwift Interpreter
         * Provides bridge between Swift UI code and C++ UI runtime
         */
        class UIIntegration {
        public:
            static UIIntegration& getInstance();
            
            // Initialize UI system
            bool initialize(int argc = 0, char** argv = nullptr);
            
            // Create UI components from Swift values
            std::shared_ptr<UIWidget> createTextFromValue(const MiniSwift::Value& value);
            std::shared_ptr<UIWidget> createButtonFromValue(const MiniSwift::Value& titleValue, const MiniSwift::Value& actionValue);
            std::shared_ptr<UIWidget> createVStackFromValue(const MiniSwift::Value& spacingValue);
            std::shared_ptr<UIWidget> createHStackFromValue(const MiniSwift::Value& spacingValue);
            
            // Convert Swift values to UI types
            Color valueToColor(const MiniSwift::Value& value);
            Font valueToFont(const MiniSwift::Value& value);
            EdgeInsets valueToEdgeInsets(const MiniSwift::Value& value);
            Size valueToSize(const MiniSwift::Value& value);
            
            // Convert UI types to Swift values
            MiniSwift::Value colorToValue(const Color& color);
            MiniSwift::Value fontToValue(const Font& font);
            MiniSwift::Value edgeInsetsToValue(const EdgeInsets& insets);
            MiniSwift::Value sizeToValue(const Size& size);
            
            // UI Application management
            void setMainView(std::shared_ptr<UIWidget> view);
            void runUIApplication();
            void quitUIApplication();
            
            // Backend selection
            enum class Backend {
                Auto,    // Automatically select best available backend
                GTK4,    // Force GTK4 backend
                Mock     // Use mock backend for testing
            };
            
            void setBackend(Backend backend);
            Backend getCurrentBackend() const;
            bool isBackendAvailable(Backend backend) const;
            
            // UI State management
            void updateUI();
            void invalidateLayout();
            
            // Event handling integration
            void registerEventHandler(const std::string& eventType, std::function<void(const MiniSwift::Value&)> handler);
            void triggerEvent(const std::string& eventType, const MiniSwift::Value& eventData);
            
            // Cleanup
            void cleanup();
            
        private:
            UIIntegration() = default;
            ~UIIntegration() = default;
            UIIntegration(const UIIntegration&) = delete;
            UIIntegration& operator=(const UIIntegration&) = delete;
            
            // Internal state
            bool initialized_ = false;
            Backend currentBackend_ = Backend::Auto;
            std::shared_ptr<UIWidget> mainView_;
            
            // Event handlers
            std::map<std::string, std::function<void(const MiniSwift::Value&)>> eventHandlers_;
            
            // Helper methods
            Backend selectBestBackend();
            std::shared_ptr<UIWidget> createWidgetForBackend(const std::string& type, const std::vector<MiniSwift::Value>& args);
            
            // Backend-specific factories
            std::shared_ptr<UIWidget> createGTK4Widget(const std::string& type, const std::vector<MiniSwift::Value>& args);
            std::shared_ptr<UIWidget> createMockWidget(const std::string& type, const std::vector<MiniSwift::Value>& args);
        };
        
        /**
         * UI Interpreter Extensions
         * Adds UI-specific functions to the MiniSwift interpreter
         */
        class UIInterpreter {
        public:
            // Register UI functions with interpreter environment
            static void registerUIFunctions(MiniSwift::Environment& env);
            
            // UI function implementations
            static MiniSwift::Value createText(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value createButton(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value createVStack(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value createHStack(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value createColor(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value createFont(const std::vector<MiniSwift::Value>& args);
            
            // UI modifier functions
            static MiniSwift::Value applyPadding(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value applyBackground(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value applyFrame(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value applyForegroundColor(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value applyFont(const std::vector<MiniSwift::Value>& args);
            
            // UI application functions
            static MiniSwift::Value runApp(const std::vector<MiniSwift::Value>& args);
            static MiniSwift::Value setMainView(const std::vector<MiniSwift::Value>& args);
            
        private:
            // Helper methods for value conversion
            static std::shared_ptr<UIWidget> extractWidget(const MiniSwift::Value& value);
            static MiniSwift::Value wrapWidget(std::shared_ptr<UIWidget> widget);
        };
        
        /**
         * UI Value Types
         * Custom value types for UI components
         */
        class UIWidgetValue {
        public:
            UIWidgetValue(std::shared_ptr<UIWidget> widget) : widget_(widget) {}
            
            std::shared_ptr<UIWidget> getWidget() const { return widget_; }
            
            // Value interface
            std::string toString() const;
            bool equals(const UIWidgetValue& other) const;
            
        private:
            std::shared_ptr<UIWidget> widget_;
        };
        
        /**
         * UI Constants and Utilities
         */
        namespace UIConstants {
            // Predefined colors
            extern const Color BLACK;
            extern const Color WHITE;
            extern const Color RED;
            extern const Color GREEN;
            extern const Color BLUE;
            extern const Color CLEAR;
            
            // Predefined fonts
            extern const Font SYSTEM_FONT;
            extern const Font TITLE_FONT;
            extern const Font CAPTION_FONT;
            
            // Default values
            extern const double DEFAULT_SPACING;
            extern const EdgeInsets DEFAULT_PADDING;
            extern const Size DEFAULT_BUTTON_SIZE;
        }
        
        /**
         * UI Error Handling
         */
        class UIError : public std::runtime_error {
        public:
            UIError(const std::string& message) : std::runtime_error("UI Error: " + message) {}
        };
        
        class UIBackendError : public UIError {
        public:
            UIBackendError(const std::string& message) : UIError("Backend Error: " + message) {}
        };
        
        class UIValueError : public UIError {
        public:
            UIValueError(const std::string& message) : UIError("Value Error: " + message) {}
        };
        
    } // namespace UI
} // namespace MiniSwift