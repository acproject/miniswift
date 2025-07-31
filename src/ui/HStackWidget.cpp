#include "HStackWidget.h"
#include <iostream>
#include <algorithm>

namespace MiniSwift {
    namespace UI {
        
        void HStackWidget::render() {
            std::cout << "[HStackWidget] Rendering HStack with " << children_.size() << " children" << std::endl;
            
            for (auto& child : children_) {
                if (child) {
                    child->render();
                }
            }
        }
        
        void HStackWidget::layout() {
            if (children_.empty()) {
                frame_.size = Size(0, 0);
                return;
            }
            
            // Layout children horizontally
            double totalWidth = 0;
            double maxHeight = 0;
            double currentX = padding_.left;
            
            for (size_t i = 0; i < children_.size(); ++i) {
                auto& child = children_[i];
                if (child) {
                    // Layout child first
                    child->layout();
                    
                    // Position child
                    Rect childFrame = child->getFrame();
                    childFrame.origin.x = currentX;
                    childFrame.origin.y = padding_.top;
                    child->setFrame(childFrame);
                    
                    // Update totals
                    maxHeight = std::max(maxHeight, childFrame.size.height);
                    currentX += childFrame.size.width;
                    
                    // Add spacing between children (except after last child)
                    if (i < children_.size() - 1) {
                        currentX += spacing_;
                    }
                }
            }
            
            totalWidth = currentX + padding_.right;
            
            // Set our frame size
            frame_.size = Size(totalWidth, maxHeight + padding_.top + padding_.bottom);
        }
        
        // Factory function
        std::shared_ptr<HStackWidget> createHStack(double spacing) {
            return std::make_shared<HStackWidget>(spacing);
        }
        
    } // namespace UI
} // namespace MiniSwift