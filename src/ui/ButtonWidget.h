#pragma once

#include "UIRuntime.h"

namespace MiniSwift {
    namespace UI {
        
        class ButtonWidget : public UIWidget {
        public:
            ButtonWidget(const std::string& title, UICallback callback = nullptr)
                : UIWidget(WidgetType::Button), title_(title), callback_(callback) {}
            
            void setTitle(const std::string& title) { title_ = title; }
            std::string getTitle() const { return title_; }
            
            void setCallback(UICallback callback) { callback_ = callback; }
            void onClick() { if (callback_) callback_(); }
            
            void render() override;
            void layout() override;
            
        private:
            std::string title_;
            UICallback callback_;
        };
        
        // Factory function
        std::shared_ptr<ButtonWidget> createButton(const std::string& title, UICallback callback = nullptr);
        
    } // namespace UI
} // namespace MiniSwift