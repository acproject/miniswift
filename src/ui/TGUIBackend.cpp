#include "TGUIBackend.h"
#include <iostream>
#include <chrono>

#if defined(HAVE_TGUI)
  #include <TGUI/TGUI.hpp>
  // TGUI 1.0 backend header for SFML integration
  #include <TGUI/Backend/SFML-Graphics.hpp>
  #include <SFML/Graphics.hpp>
  #include <SFML/Config.hpp>
  #include <variant>
#endif

namespace MiniSwift {
namespace UI {
namespace TGUI {

// ===== TGUIWidget =====
TGUIWidget::TGUIWidget(void* widget) : nativeWidget_(widget), ownsWidget_(false) {}
TGUIWidget::~TGUIWidget() = default;
void TGUIWidget::setNativeWidget(void* widget) { nativeWidget_ = widget; }

void TGUIWidget::show() {
#if defined(HAVE_TGUI)
    // Widgets are visible by default in TGUI; no-op for now
#else
    (void)0;
#endif
}
void TGUIWidget::hide() {
#if defined(HAVE_TGUI)
    // No direct hide; would need to set visible to false if supported
    // Kept as placeholder
#else
    (void)0;
#endif
}
void TGUIWidget::setSize(int width, int height) {
#if defined(HAVE_TGUI)
    if (nativeWidget_) {
        auto* shared = reinterpret_cast<tgui::Widget::Ptr*>(nativeWidget_);
        if (shared && *shared)
            (*shared)->setSize(static_cast<float>(width), static_cast<float>(height));
    }
#else
    (void)width; (void)height;
#endif
}
void TGUIWidget::setMargins(int top, int left, int bottom, int right) {
#if defined(HAVE_TGUI)
    // Margin support depends on layout used; placeholder
    (void)top; (void)left; (void)bottom; (void)right;
#else
    (void)top; (void)left; (void)bottom; (void)right;
#endif
}

// ===== TGUITextWidget =====
TGUITextWidget::TGUITextWidget(const std::string& text) : TextWidget(text) {}
TGUITextWidget::~TGUITextWidget() {
#if defined(HAVE_TGUI)
    auto* sp = reinterpret_cast<tgui::Label::Ptr*>(label_);
    delete sp;
    label_ = nullptr;
#endif
}

void TGUITextWidget::render() {
#if defined(HAVE_TGUI)
    if (!label_) {
        auto label = tgui::Label::create(getText());
        // Keep shared pointer alive inside this wrapper
        label_ = new tgui::Label::Ptr(label);
        setNativeWidget(reinterpret_cast<void*>(label_));
        ownsWidget_ = true;
    }
    updateTGUIText();
    updateTGUIFont();
    updateTGUIColor();
#else
    // No-op in mock
#endif
}

void TGUITextWidget::layout() {
    // TGUI uses automatic layout within containers; placeholder
}

void TGUITextWidget::updateTGUIText() {
#if defined(HAVE_TGUI)
    if (label_) {
        auto* sp = reinterpret_cast<tgui::Label::Ptr*>(label_);
        if (sp && *sp)
            (*sp)->setText(getText());
    }
#endif
}
void TGUITextWidget::updateTGUIFont() {
#if defined(HAVE_TGUI)
    // Mapping MiniSwift::UI::Font to TGUI font left for future work
#endif
}
void TGUITextWidget::updateTGUIColor() {
#if defined(HAVE_TGUI)
    if (label_) {
        auto* sp = reinterpret_cast<tgui::Label::Ptr*>(label_);
        if (sp && *sp) {
            auto c = getTextColor();
            (*sp)->getRenderer()->setTextColor(tgui::Color(c.red, c.green, c.blue, c.alpha));
        }
    }
#endif
}

// ===== TGUIButtonWidget =====
TGUIButtonWidget::TGUIButtonWidget(const std::string& title, UICallback callback)
    : ButtonWidget(title, callback) {}
TGUIButtonWidget::~TGUIButtonWidget() {
#if defined(HAVE_TGUI)
    auto* sp = reinterpret_cast<tgui::Button::Ptr*>(button_);
    delete sp;
    button_ = nullptr;
#endif
}

void TGUIButtonWidget::render() {
#if defined(HAVE_TGUI)
    if (!button_) {
        auto btn = tgui::Button::create(getTitle());
        button_ = new tgui::Button::Ptr(btn);
        setNativeWidget(reinterpret_cast<void*>(button_));
        ownsWidget_ = true;
        connectSignals();
    }
    updateTGUITitle();
#endif
}

void TGUIButtonWidget::layout() {
    // Default auto layout
}

void TGUIButtonWidget::updateTGUITitle() {
#if defined(HAVE_TGUI)
    if (button_) {
        auto* sp = reinterpret_cast<tgui::Button::Ptr*>(button_);
        if (sp && *sp)
            (*sp)->setText(getTitle());
    }
#endif
}

void TGUIButtonWidget::connectSignals() {
#if defined(HAVE_TGUI)
    if (button_) {
        auto* sp = reinterpret_cast<tgui::Button::Ptr*>(button_);
        if (sp && *sp) {
            (*sp)->onPress([this]{
                onClick();
            });
        }
    }
#endif
}

// ===== TGUIVStackWidget =====
TGUIVStackWidget::TGUIVStackWidget(double spacing) : VStackWidget(spacing) {}
TGUIVStackWidget::~TGUIVStackWidget() {
#if defined(HAVE_TGUI)
    auto* sp = reinterpret_cast<tgui::VerticalLayout::Ptr*>(layout_);
    delete sp;
    layout_ = nullptr;
#endif
}

void TGUIVStackWidget::render() {
#if defined(HAVE_TGUI)
    if (!layout_) {
        auto vl = tgui::VerticalLayout::create();
        layout_ = new tgui::VerticalLayout::Ptr(vl);
        setNativeWidget(reinterpret_cast<void*>(layout_));
        ownsWidget_ = true;
        updateTGUISpacing();
    }
    for (auto& child : children_) {
        if (child) child->render();
    }
#endif
}

void TGUIVStackWidget::layout() {
    // Automatic in TGUI
}

void TGUIVStackWidget::updateTGUISpacing() {
#if defined(HAVE_TGUI)
    // Spacing customization placeholder; may require custom padding or dummy spacers
#endif
}

void TGUIVStackWidget::addTGUIChild(std::shared_ptr<UIWidget> child) {
#if defined(HAVE_TGUI)
    if (!layout_ || !child) return;
    auto* vlsp = reinterpret_cast<tgui::VerticalLayout::Ptr*>(layout_);
    auto* tguiChild = dynamic_cast<TGUIWidget*>(child.get());
    if (vlsp && *vlsp && tguiChild && tguiChild->getNativeWidget()) {
        auto* childWidget = reinterpret_cast<tgui::Widget::Ptr*>(tguiChild->getNativeWidget());
        if (childWidget && *childWidget)
            (*vlsp)->add(*childWidget);
    }
#else
    (void)child;
#endif
}

// ===== TGUIHStackWidget =====
TGUIHStackWidget::TGUIHStackWidget(double spacing) : HStackWidget(spacing) {}
TGUIHStackWidget::~TGUIHStackWidget() {
#if defined(HAVE_TGUI)
    auto* sp = reinterpret_cast<tgui::HorizontalLayout::Ptr*>(layout_);
    delete sp;
    layout_ = nullptr;
#endif
}

void TGUIHStackWidget::render() {
#if defined(HAVE_TGUI)
    if (!layout_) {
        auto hl = tgui::HorizontalLayout::create();
        layout_ = new tgui::HorizontalLayout::Ptr(hl);
        setNativeWidget(reinterpret_cast<void*>(layout_));
        ownsWidget_ = true;
        updateTGUISpacing();
    }
    for (auto& child : children_) {
        if (child) child->render();
    }
#endif
}

void TGUIHStackWidget::layout() {
    // Automatic
}

void TGUIHStackWidget::updateTGUISpacing() {
#if defined(HAVE_TGUI)
    // Spacing customization placeholder
#endif
}

void TGUIHStackWidget::addTGUIChild(std::shared_ptr<UIWidget> child) {
#if defined(HAVE_TGUI)
    if (!layout_ || !child) return;
    auto* hlsp = reinterpret_cast<tgui::HorizontalLayout::Ptr*>(layout_);
    auto* tguiChild = dynamic_cast<TGUIWidget*>(child.get());
    if (hlsp && *hlsp && tguiChild && tguiChild->getNativeWidget()) {
        auto* childWidget = reinterpret_cast<tgui::Widget::Ptr*>(tguiChild->getNativeWidget());
        if (childWidget && *childWidget)
            (*hlsp)->add(*childWidget);
    }
#else
    (void)child;
#endif
}

// ===== TGUIApplication =====
TGUIApplication& TGUIApplication::getInstance() {
    static TGUIApplication inst;
    return inst;
}

bool TGUIApplication::initialize(int argc, char** argv) {
    (void)argc; (void)argv;
#if defined(HAVE_TGUI)
    if (initialized_) return true;
    // Create window and GUI (SFML backend)
    auto* win = new sf::RenderWindow(sf::VideoMode({800u, 600u}), "MiniSwift UI (TGUI)");
    window_ = win;

    auto* gui = new tgui::Gui(*win);
    gui_ = gui;

    initialized_ = true;
    return true;
#else
    std::cout << "[MiniSwift::UI] TGUI not available, initialize() noop\n";
    initialized_ = false;
    return false;
#endif
}

void TGUIApplication::createMainWindow() {
#if defined(HAVE_TGUI)
    if (!window_) {
        window_ = new sf::RenderWindow(sf::VideoMode({800u, 600u}), "MiniSwift UI (TGUI)");
    }
    if (!gui_) {
        gui_ = new tgui::Gui(*reinterpret_cast<sf::RenderWindow*>(window_));
    }
#endif
}

void TGUIApplication::setMainWindowContent(std::shared_ptr<UIWidget> content) {
#if defined(HAVE_TGUI)
    if (!gui_) {
        pendingContent_ = content;
        return;
    }
    if (!content) return;
    content->render();
    auto* gui = reinterpret_cast<tgui::Gui*>(gui_);
    auto* tguiContent = dynamic_cast<TGUIWidget*>(content.get());
    if (gui && tguiContent && tguiContent->getNativeWidget()) {
        auto* rootWidget = reinterpret_cast<tgui::Widget::Ptr*>(tguiContent->getNativeWidget());
        if (rootWidget && *rootWidget)
            gui->add(*rootWidget);
    }
#else
    (void)content;
#endif
}

void TGUIApplication::run() {
#if defined(HAVE_TGUI)
    if (!initialized_) initialize();
    createMainWindow();
    if (pendingContent_) {
        setMainWindowContent(pendingContent_);
        pendingContent_.reset();
    }
    isRunning_.store(true);
    auto* win = reinterpret_cast<sf::RenderWindow*>(window_);
    auto* gui = reinterpret_cast<tgui::Gui*>(gui_);

    while (win->isOpen() && isRunning_.load()) {
        sf::Event event;
        while (win->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                win->close();
                break;
            }
            gui->handleEvent(event);
        }
        win->clear(sf::Color(30, 30, 30));
        gui->draw();
        win->display();
    }
#else
    std::cout << "[MiniSwift::UI] TGUI not available, run() noop\n";
#endif
}

void TGUIApplication::quit() {
#if defined(HAVE_TGUI)
    isRunning_.store(false);
    auto* win = reinterpret_cast<sf::RenderWindow*>(window_);
    if (win && win->isOpen()) win->close();
#else
    (void)0;
#endif
}

void TGUIApplication::cleanup() {
#if defined(HAVE_TGUI)
    auto* gui = reinterpret_cast<tgui::Gui*>(gui_);
    delete gui;
    gui_ = nullptr;
    auto* win = reinterpret_cast<sf::RenderWindow*>(window_);
    delete win;
    window_ = nullptr;
    initialized_ = false;
#else
    (void)0;
#endif
}

// ===== Factory functions =====
std::shared_ptr<TGUITextWidget> createTGUIText(const std::string& text) {
    return std::make_shared<TGUITextWidget>(text);
}
std::shared_ptr<TGUIButtonWidget> createTGUIButton(const std::string& title, UICallback callback) {
    return std::make_shared<TGUIButtonWidget>(title, callback);
}
std::shared_ptr<TGUIVStackWidget> createTGUIVStack(double spacing) {
    return std::make_shared<TGUIVStackWidget>(spacing);
}
std::shared_ptr<TGUIHStackWidget> createTGUIHStack(double spacing) {
    return std::make_shared<TGUIHStackWidget>(spacing);
}

// ===== Availability =====
bool isTGUIAvailable() {
#if defined(HAVE_TGUI)
    return true;
#else
    return false;
#endif
}

std::string getTGUIVersion() {
#if defined(HAVE_TGUI)
    #if defined(TGUI_VERSION_MAJOR)
      return std::to_string(TGUI_VERSION_MAJOR) + "." + std::to_string(TGUI_VERSION_MINOR) + "." + std::to_string(TGUI_VERSION_PATCH);
    #else
      return "unknown";
    #endif
#else
    return "";
#endif
}

} // namespace TGUI
} // namespace UI
} // namespace MiniSwift