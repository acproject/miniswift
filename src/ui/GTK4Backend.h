#pragma once

#include "UIRuntime.h"
#include <memory>

// GTK4 forward declarations (to avoid including GTK headers in this header)
struct _GtkWidget;
struct _GtkApplication;
struct _GtkWindow;
struct _GtkBox;
struct _GtkLabel;
struct _GtkButton;

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkApplication GtkApplication;
typedef struct _GtkWindow GtkWindow;
typedef struct _GtkBox GtkBox;
typedef struct _GtkLabel GtkLabel;
typedef struct _GtkButton GtkButton;
typedef void* gpointer;

namespace MiniSwift {
    namespace UI {
        namespace GTK4 {
            
            // GTK4-specific widget wrapper
            class GTK4Widget {
            public:
                GTK4Widget(GtkWidget* widget = nullptr);
                virtual ~GTK4Widget();
                
                GtkWidget* getNativeWidget() const { return nativeWidget_; }
                void setNativeWidget(GtkWidget* widget);
                
                // Common GTK4 operations
                void show();
                void hide();
                void setSize(int width, int height);
                void setMargins(int top, int left, int bottom, int right);
                
            protected:
                GtkWidget* nativeWidget_;
                bool ownsWidget_;
            };
            
            // GTK4-specific implementations
            class GTK4TextWidget : public TextWidget, public GTK4Widget {
            public:
                GTK4TextWidget(const std::string& text);
                ~GTK4TextWidget() override = default;
                
                void render() override;
                void layout() override;
                
                // GTK4-specific methods
                void updateGTKText();
                void updateGTKFont();
                void updateGTKColor();
                
            private:
                GtkLabel* label_;
            };
            
            class GTK4ButtonWidget : public ButtonWidget, public GTK4Widget {
            public:
                GTK4ButtonWidget(const std::string& title, UICallback callback = nullptr);
                ~GTK4ButtonWidget() override = default;
                
                void render() override;
                void layout() override;
                
                // GTK4-specific methods
                void updateGTKTitle();
                void connectSignals();
                
            private:
                GtkButton* button_;
                static void onButtonClicked(GtkButton* button, gpointer userData);
            };
            
            class GTK4VStackWidget : public VStackWidget, public GTK4Widget {
            public:
                GTK4VStackWidget(double spacing = 0);
                ~GTK4VStackWidget() override = default;
                
                void render() override;
                void layout() override;
                
                // GTK4-specific methods
                void updateGTKSpacing();
                void addGTKChild(std::shared_ptr<UIWidget> child);
                
            private:
                GtkBox* box_;
            };
            
            class GTK4HStackWidget : public HStackWidget, public GTK4Widget {
            public:
                GTK4HStackWidget(double spacing = 0);
                ~GTK4HStackWidget() override = default;
                
                void render() override;
                void layout() override;
                
                // GTK4-specific methods
                void updateGTKSpacing();
                void addGTKChild(std::shared_ptr<UIWidget> child);
                
            private:
                GtkBox* box_;
            };
            
            // GTK4 Application manager
            class GTK4Application {
            public:
                static GTK4Application& getInstance();
                
                bool initialize(int argc = 0, char** argv = nullptr);
                void run();
                void quit();
                void cleanup();
                
                // Window management
                void createMainWindow();
                void setMainWindowContent(std::shared_ptr<UIWidget> content);
                
                GtkApplication* getGtkApp() const { return gtkApp_; }
                GtkWindow* getMainWindow() const { return mainWindow_; }
                
            private:
                GTK4Application() = default;
                
                GtkApplication* gtkApp_ = nullptr;
                GtkWindow* mainWindow_ = nullptr;
                bool initialized_ = false;
                
                // GTK4 callbacks
                static void onActivate(GtkApplication* app, gpointer userData);
                static void onShutdown(GtkApplication* app, gpointer userData);
            };
            
            // GTK4-specific factory functions
            std::shared_ptr<GTK4TextWidget> createGTK4Text(const std::string& text);
            std::shared_ptr<GTK4ButtonWidget> createGTK4Button(const std::string& title, UICallback callback = nullptr);
            std::shared_ptr<GTK4VStackWidget> createGTK4VStack(double spacing = 0);
            std::shared_ptr<GTK4HStackWidget> createGTK4HStack(double spacing = 0);
            
            // Utility functions
            bool isGTK4Available();
            std::string getGTK4Version();
            
            // Color conversion utilities
            void applyColorToWidget(GtkWidget* widget, const Color& color, const std::string& cssClass = "");
            std::string colorToCSSString(const Color& color);
            
        } // namespace GTK4
    } // namespace UI
} // namespace MiniSwift