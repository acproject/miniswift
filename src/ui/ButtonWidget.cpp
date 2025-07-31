#include "ButtonWidget.h"
#include <iostream>

namespace MiniSwift {
    namespace UI {
        
        void ButtonWidget::render() {
            std::cout << "[ButtonWidget] Rendering button: \"" << title_ << "\"" << std::endl;
        }
        
        void ButtonWidget::layout() {
            // Calculate button size based on title and padding
            double buttonWidth = title_.length() * 14.0 * 0.6 + 20; // Text + padding
            double buttonHeight = 44.0; // Standard button height
            
            Size contentSize(buttonWidth, buttonHeight);
            Size totalSize(contentSize.width + padding_.left + padding_.right,
                          contentSize.height + padding_.top + padding_.bottom);
            
            frame_.size = totalSize;
        }
        
        // Factory function
        std::shared_ptr<ButtonWidget> createButton(const std::string& title, UICallback callback) {
            return std::make_shared<ButtonWidget>(title, callback);
        }
        
    } // namespace UI
} // namespace MiniSwift