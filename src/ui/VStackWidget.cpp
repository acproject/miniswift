#include "VStackWidget.h"
#include <iostream>
#include <algorithm>

namespace MiniSwift {
    namespace UI {
        
        void VStackWidget::render() {
            std::cout << "[VStackWidget] Rendering VStack with " << children_.size() << " children" << std::endl;
            
            for (auto& child : children_) {
                if (child) {
                    child->render();
                }
            }
        }
        
        void VStackWidget::layout() {
            if (children_.empty()) {
                frame_.size = Size(0, 0);
                return;
            }
            
            // Layout children vertically
            double totalHeight = 0;
            double maxWidth = 0;
            double currentY = padding_.top;
            
            for (size_t i = 0; i < children_.size(); ++i) {
                auto& child = children_[i];
                if (child) {
                    // Layout child first
                    child->layout();
                    
                    // Position child
                    Rect childFrame = child->getFrame();
                    childFrame.origin.x = padding_.left;
                    childFrame.origin.y = currentY;
                    child->setFrame(childFrame);
                    
                    // Update totals
                    maxWidth = std::max(maxWidth, childFrame.size.width);
                    currentY += childFrame.size.height;
                    
                    // Add spacing between children (except after last child)
                    if (i < children_.size() - 1) {
                        currentY += spacing_;
                    }
                }
            }
            
            totalHeight = currentY + padding_.bottom;
            
            // Set our frame size
            frame_.size = Size(maxWidth + padding_.left + padding_.right, totalHeight);
        }
        
        // Factory function
        std::shared_ptr<VStackWidget> createVStack(double spacing) {
            return std::make_shared<VStackWidget>(spacing);
        }
        
    } // namespace UI
} // namespace MiniSwift