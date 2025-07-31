#include "TextWidget.h"
#include <iostream>

namespace MiniSwift {
    namespace UI {
        
        void TextWidget::render() {
            std::cout << "[TextWidget] Rendering text: \"" << text_ << "\"" << std::endl;
        }
        
        void TextWidget::layout() {
            // Calculate text size based on content and font
            double textWidth = text_.length() * font_.size * 0.6; // Approximate character width
            double textHeight = font_.size * 1.2; // Line height
            
            Size contentSize(textWidth, textHeight);
            Size totalSize(contentSize.width + padding_.left + padding_.right,
                          contentSize.height + padding_.top + padding_.bottom);
            
            frame_.size = totalSize;
        }
        
        // Factory function
        std::shared_ptr<TextWidget> createText(const std::string& text) {
            return std::make_shared<TextWidget>(text);
        }
        
    } // namespace UI
} // namespace MiniSwift