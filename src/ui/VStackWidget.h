#pragma once

#include "UIRuntime.h"

namespace MiniSwift {
    namespace UI {
        
        class VStackWidget : public UIWidget {
        public:
            VStackWidget(double spacing = 0) : UIWidget(WidgetType::VStack), spacing_(spacing) {}
            
            void setSpacing(double spacing) { spacing_ = spacing; }
            double getSpacing() const { return spacing_; }
            
            void render() override;
            void layout() override;
            
        private:
            double spacing_;
        };
        
        // Factory function
        std::shared_ptr<VStackWidget> createVStack(double spacing = 0);
        
    } // namespace UI
} // namespace MiniSwift