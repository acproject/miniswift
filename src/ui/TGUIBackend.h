#pragma once

#include "UIRuntime.h"
#include "TextWidget.h"
#include "ButtonWidget.h"
#include "VStackWidget.h"
#include "HStackWidget.h"
#include <memory>
#include <thread>
#include <atomic>
#include <future>

// Note: This header is safe to include even when TGUI isn't installed.
// All concrete TGUI types are hidden behind void* and only defined in the .cpp
// which is compiled conditionally when HAVE_TGUI is enabled.

namespace MiniSwift {
    namespace UI {
        namespace TGUI {

            // Generic TGUI widget wrapper (native pointer hidden)
            class TGUIWidget {
            public:
                TGUIWidget(void* widget = nullptr);
                virtual ~TGUIWidget();

                void* getNativeWidget() const { return nativeWidget_; }
                void setNativeWidget(void* widget);

                // Common operations (implemented in .cpp)
                void show();
                void hide();
                void setSize(int width, int height);
                void setMargins(int top, int left, int bottom, int right);

            protected:
                void* nativeWidget_ = nullptr;
                bool ownsWidget_ = false;
            };

            class TGUITextWidget : public TextWidget, public TGUIWidget {
            public:
                TGUITextWidget(const std::string& text);
                ~TGUITextWidget() override;

                void render() override;
                void layout() override;

                // TGUI-specific helpers
                void updateTGUIText();
                void updateTGUIFont();
                void updateTGUIColor();

            private:
                void* label_ = nullptr; // tgui::Label::Ptr in implementation
            };

            class TGUIButtonWidget : public ButtonWidget, public TGUIWidget {
            public:
                TGUIButtonWidget(const std::string& title, UICallback callback = nullptr);
                ~TGUIButtonWidget() override;

                void render() override;
                void layout() override;

                void updateTGUITitle();
                void connectSignals();

            private:
                void* button_ = nullptr; // tgui::Button::Ptr in implementation
                static void onButtonPressed(void* userData); // bridge
            };

            class TGUIVStackWidget : public VStackWidget, public TGUIWidget {
            public:
                TGUIVStackWidget(double spacing = 0);
                ~TGUIVStackWidget() override;

                void render() override;
                void layout() override;

                void updateTGUISpacing();
                void addTGUIChild(std::shared_ptr<UIWidget> child);

            private:
                void* layout_ = nullptr; // tgui::VerticalLayout::Ptr
            };

            class TGUIHStackWidget : public HStackWidget, public TGUIWidget {
            public:
                TGUIHStackWidget(double spacing = 0);
                ~TGUIHStackWidget() override;

                void render() override;
                void layout() override;

                void updateTGUISpacing();
                void addTGUIChild(std::shared_ptr<UIWidget> child);

            private:
                void* layout_ = nullptr; // tgui::HorizontalLayout::Ptr
            };

            // TGUI Application manager (creates window & gui in .cpp when HAVE_TGUI)
            class TGUIApplication {
            public:
                static TGUIApplication& getInstance();

                bool initialize(int argc = 0, char** argv = nullptr);
                void run();
                void quit();
                void cleanup();

                void createMainWindow();
                void setMainWindowContent(std::shared_ptr<UIWidget> content);

                bool isInitialized() const { return initialized_; }

            private:
                TGUIApplication() = default;

                bool initialized_ = false;
                std::shared_ptr<UIWidget> pendingContent_;

                // Hidden native pointers (e.g., sf::RenderWindow*, tgui::Gui*)
                void* window_ = nullptr;
                void* gui_ = nullptr;

                std::thread guiThread_;
                std::atomic<bool> isRunning_{false};
            };

            // Factory functions
            std::shared_ptr<TGUITextWidget> createTGUIText(const std::string& text);
            std::shared_ptr<TGUIButtonWidget> createTGUIButton(const std::string& title, UICallback callback = nullptr);
            std::shared_ptr<TGUIVStackWidget> createTGUIVStack(double spacing = 0);
            std::shared_ptr<TGUIHStackWidget> createTGUIHStack(double spacing = 0);

            // Availability
            bool isTGUIAvailable();
            std::string getTGUIVersion();

        } // namespace TGUI
    } // namespace UI
} // namespace MiniSwift