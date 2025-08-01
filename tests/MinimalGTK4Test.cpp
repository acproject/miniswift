#include <iostream>
#include <memory>
#include "../src/ui/UIIntegration.h"
#include "../src/ui/GTK4Backend.h"

int main() {
    std::cout << "Starting Minimal GTK4 Layout Test..." << std::endl;
    
    try {
        // Initialize UI system
        auto& uiIntegration = MiniSwift::UI::UIIntegration::getInstance();
        if (!uiIntegration.initialize()) {
            std::cerr << "Failed to initialize UI system" << std::endl;
            return 1;
        }
        
        // Create UI components using GTK4 factory functions
        using namespace MiniSwift::UI::GTK4;
        
        auto titleText = createGTK4Text("GTK4 Layout Test");
        auto descText = createGTK4Text("Testing VStack and HStack layouts");
        
        auto button1 = createGTK4Button("Button 1");
        auto button2 = createGTK4Button("Button 2");
        
        auto statusText = createGTK4Text("Status: Layout components created");
        
        // Create layout containers
        auto buttonStack = createGTK4HStack(15); // horizontal stack with 15px spacing
        auto mainStack = createGTK4VStack(20);   // vertical stack with 20px spacing
        
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
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Application finished successfully" << std::endl;
    return 0;
}