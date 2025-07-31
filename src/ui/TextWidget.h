#pragma once

#include "UIRuntime.h"

namespace MiniSwift {
    namespace UI {
        
        class TextWidget : public UIWidget {
        public:
            TextWidget(const std::string& text) 
                : UIWidget(WidgetType::Text), text_(text) {}
            
            void setText(const std::string& text) { text_ = text; }
            std::string getText() const { return text_; }
            
            void setFont(const Font& font) { font_ = font; }
            Font getFont() const { return font_; }
            
            void setTextColor(const Color& color) { textColor_ = color; }
            Color getTextColor() const { return textColor_; }
            
            void render() override;
            void layout() override;
            
        private:
            std::string text_;
            Font font_ = Font::body();
            Color textColor_ = Color::black();
        };
        
        // Factory function
        std::shared_ptr<TextWidget> createText(const std::string& text);
        
    } // namespace UI
} // namespace MiniSwift