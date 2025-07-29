#include "UIRuntime.h"
#include <iostream>
#include <algorithm>

namespace MiniSwift {
    namespace UI {
        
        // TextWidget implementation
        void TextWidget::render() {
            // Platform-specific text rendering will be implemented here
            // For now, just output to console for debugging
            std::cout << "[TextWidget] Rendering text: \"" << text_ << "\"" << std::endl;
        }
        
        void TextWidget::layout() {
            // Calculate text size based on font and content
            // This is a simplified implementation
            double textWidth = text_.length() * font_.size * 0.6; // Rough estimation
            double textHeight = font_.size * 1.2;
            
            Size contentSize(textWidth, textHeight);
            Size totalSize(contentSize.width + padding_.left + padding_.right,
                          contentSize.height + padding_.top + padding_.bottom);
            
            frame_.size = totalSize;
        }
        
        // ButtonWidget implementation
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
        
        // VStackWidget implementation
        void VStackWidget::render() {
            std::cout << "[VStackWidget] Rendering VStack with " << children_.size() << " children" << std::endl;
            
            // Render all children
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
                if (!child) continue;
                
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
                
                // Add spacing (except for last child)
                if (i < children_.size() - 1) {
                    currentY += spacing_;
                }
            }
            
            totalHeight = currentY + padding_.bottom;
            
            // Set our frame size
            frame_.size = Size(maxWidth + padding_.left + padding_.right, totalHeight);
        }
        
        // HStackWidget implementation
        void HStackWidget::render() {
            std::cout << "[HStackWidget] Rendering HStack with " << children_.size() << " children" << std::endl;
            
            // Render all children
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
                if (!child) continue;
                
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
                
                // Add spacing (except for last child)
                if (i < children_.size() - 1) {
                    currentX += spacing_;
                }
            }
            
            totalWidth = currentX + padding_.right;
            
            // Set our frame size
            frame_.size = Size(totalWidth, maxHeight + padding_.top + padding_.bottom);
        }
        
        // UIApplication implementation
        UIApplication& UIApplication::getInstance() {
            static UIApplication instance;
            return instance;
        }
        
        void UIApplication::setRootWidget(std::shared_ptr<UIWidget> root) {
            rootWidget_ = root;
        }
        
        bool UIApplication::initialize() {
            std::cout << "[UIApplication] Initializing UI system..." << std::endl;
            
            // Platform-specific initialization will go here
            // For now, just return true
            return true;
        }
        
        void UIApplication::run() {
            if (!initialize()) {
                std::cerr << "[UIApplication] Failed to initialize UI system" << std::endl;
                return;
            }
            
            if (!rootWidget_) {
                std::cerr << "[UIApplication] No root widget set" << std::endl;
                return;
            }
            
            std::cout << "[UIApplication] Starting UI application..." << std::endl;
            running_ = true;
            
            // Layout and render the root widget
            rootWidget_->layout();
            rootWidget_->render();
            
            std::cout << "[UIApplication] UI application started successfully" << std::endl;
            std::cout << "[UIApplication] Root widget frame: " 
                      << rootWidget_->getFrame().size.width << "x" 
                      << rootWidget_->getFrame().size.height << std::endl;
            
            // In a real implementation, this would start the event loop
            // For now, we just simulate a simple run
        }
        
        void UIApplication::quit() {
            std::cout << "[UIApplication] Quitting UI application..." << std::endl;
            running_ = false;
            cleanup();
        }
        
        void UIApplication::cleanup() {
            std::cout << "[UIApplication] Cleaning up UI system..." << std::endl;
            rootWidget_.reset();
        }
        
        // Factory functions
        std::shared_ptr<TextWidget> createText(const std::string& text) {
            return std::make_shared<TextWidget>(text);
        }
        
        std::shared_ptr<ButtonWidget> createButton(const std::string& title, UICallback callback) {
            return std::make_shared<ButtonWidget>(title, callback);
        }
        
        std::shared_ptr<VStackWidget> createVStack(double spacing) {
            return std::make_shared<VStackWidget>(spacing);
        }
        
        std::shared_ptr<HStackWidget> createHStack(double spacing) {
            return std::make_shared<HStackWidget>(spacing);
        }
        
    } // namespace UI
} // namespace MiniSwift