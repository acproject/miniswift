#include <iostream>
#include <memory>
#include "../src/ui/UIIntegration.h"
#include "../src/ui/TGUIBackend.h"

int main() {
    std::cout << "Starting Minimal TGUI Layout Test..." << std::endl;
    
    try {
        // Initialize UI system with TGUI backend
        auto& uiIntegration = MiniSwift::UI::UIIntegration::getInstance();
        uiIntegration.setBackend(MiniSwift::UI::UIIntegration::Backend::TGUI);
        if (!uiIntegration.initialize()) {
            std::cerr << "Failed to initialize UI system" << std::endl;
            return 1;
        }
        
        // Create UI components using TGUI factory functions
        using namespace MiniSwift::UI::TGUI;
        
        auto titleText = createTGUIText("TGUI Layout Test");
        auto descText = createTGUIText("Testing VStack and HStack layouts");
        
        auto button1 = createTGUIButton("Button 1");
        auto button2 = createTGUIButton("Button 2");
        
        auto statusText = createTGUIText("Status: Layout components created");
        
        // Create layout containers
        auto buttonStack = createTGUIHStack(15); // horizontal stack with 15px spacing
        auto mainStack = createTGUIVStack(20);   // vertical stack with 20px spacing
        
        // Add buttons to horizontal stack
        uiIntegration.addChildToWidget(buttonStack, button1);
        uiIntegration.addChildToWidget(buttonStack, button2);
        
        // Add all components to main vertical stack
        uiIntegration.addChildToWidget(mainStack, titleText);
        uiIntegration.addChildToWidget(mainStack, descText);
        uiIntegration.addChildToWidget(mainStack, buttonStack);
        uiIntegration.addChildToWidget(mainStack, statusText);
        
        std::cout << "UI layout components created successfully" << std::endl;
        
        // Set the main view and run the application
        uiIntegration.setMainView(mainStack);
        
        std::cout << "Starting UI application with layout..." << std::endl;
        
        // Run the UI application
        uiIntegration.runUIApplication();
        
        std::cout << "UI application finished" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}