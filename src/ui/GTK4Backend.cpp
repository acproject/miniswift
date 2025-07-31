#include "GTK4Backend.h"
#include "../interpreter/Concurrency/ConcurrencyRuntime.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <future>

// Only include GTK headers if available
#ifdef HAVE_GTK4
#include <gtk/gtk.h>
#else
// Mock GTK4 functions for compilation without GTK4
#define gtk_application_new(id, flags) nullptr
#define gtk_application_window_new(app) nullptr
#define gtk_window_set_title(window, title)
#define gtk_window_set_default_size(window, width, height)
#define gtk_window_present(window)
#define gtk_label_new(text) nullptr
#define gtk_button_new_with_label(label) nullptr
#define gtk_box_new(orientation, spacing) nullptr
#define gtk_box_append(box, child)
#define gtk_window_set_child(window, child)
#define g_signal_connect(instance, signal, callback, data) 0
#define g_application_run(app, argc, argv) 0
#define g_object_unref(object)
#define gtk_widget_show(widget)
#define gtk_widget_hide(widget)
#define gtk_widget_set_size_request(widget, width, height)
#define gtk_widget_set_margin_top(widget, margin)
#define gtk_widget_set_margin_bottom(widget, margin)
#define gtk_widget_set_margin_start(widget, margin)
#define gtk_widget_set_margin_end(widget, margin)
#define gtk_label_set_text(label, text)
#define gtk_button_set_label(button, label)
#define gtk_box_set_spacing(box, spacing)
#define GTK_ORIENTATION_VERTICAL 0
#define GTK_ORIENTATION_HORIZONTAL 1
#define G_APPLICATION_DEFAULT_FLAGS 0
#endif

namespace MiniSwift {
    namespace UI {
        namespace GTK4 {
            
            // GTK4Widget implementation
            GTK4Widget::GTK4Widget(GtkWidget* widget) 
                : nativeWidget_(widget), ownsWidget_(false) {
            }
            
            GTK4Widget::~GTK4Widget() {
                if (ownsWidget_ && nativeWidget_) {
#ifdef HAVE_GTK4
                    g_object_unref(nativeWidget_);
#endif
                }
            }
            
            void GTK4Widget::setNativeWidget(GtkWidget* widget) {
                if (ownsWidget_ && nativeWidget_) {
#ifdef HAVE_GTK4
                    g_object_unref(nativeWidget_);
#endif
                }
                nativeWidget_ = widget;
                ownsWidget_ = true;
            }
            
            void GTK4Widget::show() {
                if (nativeWidget_) {
#ifdef HAVE_GTK4
                    gtk_widget_show(nativeWidget_);
#else
                    std::cout << "[GTK4Widget] Show widget (mock)" << std::endl;
#endif
                }
            }
            
            void GTK4Widget::hide() {
                if (nativeWidget_) {
#ifdef HAVE_GTK4
                    gtk_widget_hide(nativeWidget_);
#else
                    std::cout << "[GTK4Widget] Hide widget (mock)" << std::endl;
#endif
                }
            }
            
            void GTK4Widget::setSize(int width, int height) {
                if (nativeWidget_) {
#ifdef HAVE_GTK4
                    gtk_widget_set_size_request(nativeWidget_, width, height);
#else
                    std::cout << "[GTK4Widget] Set size: " << width << "x" << height << " (mock)" << std::endl;
#endif
                }
            }
            
            void GTK4Widget::setMargins(int top, int left, int bottom, int right) {
                if (nativeWidget_) {
#ifdef HAVE_GTK4
                    gtk_widget_set_margin_top(nativeWidget_, top);
                    gtk_widget_set_margin_bottom(nativeWidget_, bottom);
                    gtk_widget_set_margin_start(nativeWidget_, left);
                    gtk_widget_set_margin_end(nativeWidget_, right);
#else
                    std::cout << "[GTK4Widget] Set margins: " << top << "," << left 
                              << "," << bottom << "," << right << " (mock)" << std::endl;
#endif
                }
            }
            
            // GTK4TextWidget implementation
            GTK4TextWidget::GTK4TextWidget(const std::string& text) 
                : TextWidget(text), GTK4Widget() {
#ifdef HAVE_GTK4
                label_ = GTK_LABEL(gtk_label_new(text.c_str()));
                setNativeWidget(GTK_WIDGET(label_));
#else
                label_ = nullptr;
                std::cout << "[GTK4TextWidget] Created text widget: \"" << text << "\" (mock)" << std::endl;
#endif
            }
            
            void GTK4TextWidget::render() {
                updateGTKText();
                updateGTKFont();
                updateGTKColor();
                show();
                
                std::cout << "[GTK4TextWidget] Rendered text: \"" << getText() << "\"" << std::endl;
            }
            
            void GTK4TextWidget::layout() {
                TextWidget::layout(); // Call base class layout
                
                // Apply size to GTK widget
                Rect frame = getFrame();
                setSize(static_cast<int>(frame.size.width), static_cast<int>(frame.size.height));
                
                // Apply margins
                EdgeInsets padding = getPadding();
                setMargins(static_cast<int>(padding.top), static_cast<int>(padding.left),
                          static_cast<int>(padding.bottom), static_cast<int>(padding.right));
            }
            
            void GTK4TextWidget::updateGTKText() {
                if (label_) {
#ifdef HAVE_GTK4
                    gtk_label_set_text(label_, getText().c_str());
#endif
                }
            }
            
            void GTK4TextWidget::updateGTKFont() {
                // Font styling would be implemented here using CSS providers
                // For now, just log the font information
                Font font = getFont();
                std::cout << "[GTK4TextWidget] Font: " << font.family << ", size: " << font.size << std::endl;
            }
            
            void GTK4TextWidget::updateGTKColor() {
                // Color styling would be implemented here using CSS providers
                Color color = getTextColor();
                std::cout << "[GTK4TextWidget] Color: RGBA(" << color.red << ", " << color.green 
                          << ", " << color.blue << ", " << color.alpha << ")" << std::endl;
            }
            
            // GTK4ButtonWidget implementation
            GTK4ButtonWidget::GTK4ButtonWidget(const std::string& title, UICallback callback) 
                : ButtonWidget(title, callback), GTK4Widget() {
#ifdef HAVE_GTK4
                button_ = GTK_BUTTON(gtk_button_new_with_label(title.c_str()));
                setNativeWidget(GTK_WIDGET(button_));
                connectSignals();
#else
                button_ = nullptr;
                std::cout << "[GTK4ButtonWidget] Created button: \"" << title << "\" (mock)" << std::endl;
#endif
            }
            
            void GTK4ButtonWidget::render() {
                updateGTKTitle();
                show();
                
                std::cout << "[GTK4ButtonWidget] Rendered button: \"" << getTitle() << "\"" << std::endl;
            }
            
            void GTK4ButtonWidget::layout() {
                ButtonWidget::layout(); // Call base class layout
                
                // Apply size to GTK widget
                Rect frame = getFrame();
                setSize(static_cast<int>(frame.size.width), static_cast<int>(frame.size.height));
                
                // Apply margins
                EdgeInsets padding = getPadding();
                setMargins(static_cast<int>(padding.top), static_cast<int>(padding.left),
                          static_cast<int>(padding.bottom), static_cast<int>(padding.right));
            }
            
            void GTK4ButtonWidget::updateGTKTitle() {
                if (button_) {
#ifdef HAVE_GTK4
                    gtk_button_set_label(button_, getTitle().c_str());
#endif
                }
            }
            
            void GTK4ButtonWidget::connectSignals() {
                if (button_) {
#ifdef HAVE_GTK4
                    g_signal_connect(button_, "clicked", G_CALLBACK(onButtonClicked), this);
#endif
                }
            }
            
            void GTK4ButtonWidget::onButtonClicked(GtkButton* button, gpointer userData) {
                auto* buttonWidget = static_cast<GTK4ButtonWidget*>(userData);
                if (buttonWidget) {
                    buttonWidget->onClick();
                }
            }
            
            // GTK4VStackWidget implementation
            GTK4VStackWidget::GTK4VStackWidget(double spacing) 
                : VStackWidget(spacing), GTK4Widget() {
#ifdef HAVE_GTK4
                box_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, static_cast<int>(spacing)));
                setNativeWidget(GTK_WIDGET(box_));
#else
                box_ = nullptr;
                std::cout << "[GTK4VStackWidget] Created VStack with spacing: " << spacing << " (mock)" << std::endl;
#endif
            }
            
            void GTK4VStackWidget::render() {
                updateGTKSpacing();
                
                // Add children to GTK box
                for (auto& child : getChildren()) {
                    addGTKChild(child);
                    if (child) {
                        child->render();
                    }
                }
                
                show();
                std::cout << "[GTK4VStackWidget] Rendered VStack with " << getChildren().size() << " children" << std::endl;
            }
            
            void GTK4VStackWidget::layout() {
                VStackWidget::layout(); // Call base class layout
                
                // Apply size to GTK widget
                Rect frame = getFrame();
                setSize(static_cast<int>(frame.size.width), static_cast<int>(frame.size.height));
            }
            
            void GTK4VStackWidget::updateGTKSpacing() {
                if (box_) {
#ifdef HAVE_GTK4
                    gtk_box_set_spacing(box_, static_cast<int>(getSpacing()));
#endif
                }
            }
            
            void GTK4VStackWidget::addGTKChild(std::shared_ptr<UIWidget> child) {
                if (!box_ || !child) return;
                
                // Try to get GTK4 widget from child
                auto gtk4Child = std::dynamic_pointer_cast<GTK4Widget>(child);
                if (gtk4Child && gtk4Child->getNativeWidget()) {
#ifdef HAVE_GTK4
                    gtk_box_append(box_, gtk4Child->getNativeWidget());
#endif
                }
            }
            
            // GTK4HStackWidget implementation
            GTK4HStackWidget::GTK4HStackWidget(double spacing) 
                : HStackWidget(spacing), GTK4Widget() {
#ifdef HAVE_GTK4
                box_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, static_cast<int>(spacing)));
                setNativeWidget(GTK_WIDGET(box_));
#else
                box_ = nullptr;
                std::cout << "[GTK4HStackWidget] Created HStack with spacing: " << spacing << " (mock)" << std::endl;
#endif
            }
            
            void GTK4HStackWidget::render() {
                updateGTKSpacing();
                
                // Add children to GTK box
                for (auto& child : getChildren()) {
                    addGTKChild(child);
                    if (child) {
                        child->render();
                    }
                }
                
                show();
                std::cout << "[GTK4HStackWidget] Rendered HStack with " << getChildren().size() << " children" << std::endl;
            }
            
            void GTK4HStackWidget::layout() {
                HStackWidget::layout(); // Call base class layout
                
                // Apply size to GTK widget
                Rect frame = getFrame();
                setSize(static_cast<int>(frame.size.width), static_cast<int>(frame.size.height));
            }
            
            void GTK4HStackWidget::updateGTKSpacing() {
                if (box_) {
#ifdef HAVE_GTK4
                    gtk_box_set_spacing(box_, static_cast<int>(getSpacing()));
#endif
                }
            }
            
            void GTK4HStackWidget::addGTKChild(std::shared_ptr<UIWidget> child) {
                if (!box_ || !child) return;
                
                // Try to get GTK4 widget from child
                auto gtk4Child = std::dynamic_pointer_cast<GTK4Widget>(child);
                if (gtk4Child && gtk4Child->getNativeWidget()) {
#ifdef HAVE_GTK4
                    gtk_box_append(box_, gtk4Child->getNativeWidget());
#endif
                }
            }
            
            // GTK4Application implementation
            GTK4Application& GTK4Application::getInstance() {
                static GTK4Application instance;
                return instance;
            }
            
            bool GTK4Application::initialize(int argc, char** argv) {
                if (initialized_) {
                    std::cout << "[GTK4Application] Already initialized, returning true" << std::endl;
                    return true;
                }
                
                std::cout << "[GTK4Application] Starting GTK4 initialization..." << std::endl;
                std::cout << "[GTK4Application] Thread ID: " << std::this_thread::get_id() << std::endl;
                std::cout << "[GTK4Application] argc: " << argc << std::endl;
                
                // 初始化并发运行时
                try {
                    std::cout << "[GTK4Application] Initializing concurrency runtime..." << std::endl;
                    miniswift::initialize_concurrency_runtime();
                    std::cout << "[GTK4Application] Concurrency runtime initialized successfully" << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "[GTK4Application] Failed to initialize concurrency runtime: " << e.what() << std::endl;
                    // Continue without concurrency runtime
                }
                
#ifdef HAVE_GTK4
                std::cout << "[GTK4Application] GTK4 is available, proceeding with real initialization" << std::endl;
                
                // Use async initialization with timeout to prevent hanging
                try {
                    auto& runtime = miniswift::ConcurrencyRuntime::get_instance();
                    
                    auto init_future = runtime.async(miniswift::TaskPriority::UserInteractive, [this]() {
                        std::cout << "[GTK4Application] GTK4 init task running on thread: " << std::this_thread::get_id() << std::endl;
                        
                        std::cout << "[GTK4Application] Creating GTK application..." << std::endl;
                        gtkApp_ = gtk_application_new("com.miniswift.ui", G_APPLICATION_DEFAULT_FLAGS);
                        
                        if (!gtkApp_) {
                            std::cerr << "[GTK4Application] Failed to create GTK application" << std::endl;
                            throw std::runtime_error("Failed to create GTK application");
                        }
                        
                        std::cout << "[GTK4Application] GTK application created successfully" << std::endl;
                        std::cout << "[GTK4Application] Connecting signals..." << std::endl;
                        
                        g_signal_connect(gtkApp_, "activate", G_CALLBACK(onActivate), this);
                        g_signal_connect(gtkApp_, "shutdown", G_CALLBACK(onShutdown), this);
                        
                        std::cout << "[GTK4Application] Signals connected successfully" << std::endl;
                    });
                    
                    // Wait for initialization with timeout
                    std::cout << "[GTK4Application] Waiting for GTK4 initialization (5 second timeout)..." << std::endl;
                    auto status = init_future.wait_for(std::chrono::seconds(5));
                    
                    if (status == std::future_status::timeout) {
                        std::cerr << "[GTK4Application] GTK4 initialization timed out!" << std::endl;
                        std::cerr << "[GTK4Application] This usually indicates GTK4 is not properly installed or configured" << std::endl;
                        return false;
                    } else if (status == std::future_status::ready) {
                        try {
                            init_future.get(); // This will throw if there was an exception
                            std::cout << "[GTK4Application] GTK4 initialization completed successfully" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "[GTK4Application] GTK4 initialization failed: " << e.what() << std::endl;
                            return false;
                        }
                    }
                    
                } catch (const std::exception& e) {
                    std::cerr << "[GTK4Application] Error during async GTK4 initialization: " << e.what() << std::endl;
                    // Fallback to synchronous initialization
                    std::cout << "[GTK4Application] Falling back to synchronous GTK4 initialization..." << std::endl;
                    
                    gtkApp_ = gtk_application_new("com.miniswift.ui", G_APPLICATION_DEFAULT_FLAGS);
                    if (!gtkApp_) {
                        std::cerr << "[GTK4Application] Failed to create GTK application (fallback)" << std::endl;
                        return false;
                    }
                    
                    g_signal_connect(gtkApp_, "activate", G_CALLBACK(onActivate), this);
                    g_signal_connect(gtkApp_, "shutdown", G_CALLBACK(onShutdown), this);
                }
                
#else
                std::cout << "[GTK4Application] GTK4 not available (HAVE_GTK4 not defined)" << std::endl;
                std::cout << "[GTK4Application] Using mock implementation" << std::endl;
                std::cout << "[GTK4Application] To enable GTK4, install GTK4 development libraries and recompile" << std::endl;
                gtkApp_ = nullptr;
#endif
                
                initialized_ = true;
                std::cout << "[GTK4Application] GTK4Application initialization completed" << std::endl;
                return true;
            }
            
            void GTK4Application::createMainWindow() {
                if (!gtkApp_) return;
                
#ifdef HAVE_GTK4
                mainWindow_ = GTK_WINDOW(gtk_application_window_new(gtkApp_));
                gtk_window_set_title(mainWindow_, "MiniSwift UI");
                gtk_window_set_default_size(mainWindow_, 800, 600);
#else
                std::cout << "[GTK4Application] Created main window (mock)" << std::endl;
#endif
            }
            
            void GTK4Application::setMainWindowContent(std::shared_ptr<UIWidget> content) {
                if (!mainWindow_ || !content) return;
                
                auto gtk4Content = std::dynamic_pointer_cast<GTK4Widget>(content);
                if (gtk4Content && gtk4Content->getNativeWidget()) {
#ifdef HAVE_GTK4
                    gtk_window_set_child(mainWindow_, gtk4Content->getNativeWidget());
#endif
                    std::cout << "[GTK4Application] Set main window content" << std::endl;
                }
            }
            
            void GTK4Application::run() {
                std::cout << "[GTK4Application] Starting run() method..." << std::endl;
                std::cout << "[GTK4Application] Thread ID: " << std::this_thread::get_id() << std::endl;
                
                if (!gtkApp_) {
                    std::cout << "[GTK4Application] Running in mock mode (gtkApp_ is nullptr)" << std::endl;
                    std::cout << "[GTK4Application] This means GTK4 is not available or initialization failed" << std::endl;
                    std::cout << "[GTK4Application] Mock mode completed successfully" << std::endl;
                    return;
                }
                
#ifdef HAVE_GTK4
                std::cout << "[GTK4Application] GTK4 is available, starting main loop..." << std::endl;
                std::cout << "[GTK4Application] About to call g_application_run()..." << std::endl;
                
                // Use async execution with timeout to prevent hanging
                try {
                    auto& runtime = miniswift::ConcurrencyRuntime::get_instance();
                    
                    auto run_future = runtime.async(miniswift::TaskPriority::UserInteractive, [this]() {
                        std::cout << "[GTK4Application] GTK4 main loop task running on thread: " << std::this_thread::get_id() << std::endl;
                        std::cout << "[GTK4Application] Calling g_application_run()..." << std::endl;
                        
                        int result = g_application_run(G_APPLICATION(gtkApp_), 0, nullptr);
                        
                        std::cout << "[GTK4Application] g_application_run() returned: " << result << std::endl;
                        return result;
                    });
                    
                    // Wait for GTK main loop with timeout
                    std::cout << "[GTK4Application] Waiting for GTK4 main loop (10 second timeout)..." << std::endl;
                    auto status = run_future.wait_for(std::chrono::seconds(10));
                    
                    if (status == std::future_status::timeout) {
                        std::cerr << "[GTK4Application] GTK4 main loop timed out!" << std::endl;
                        std::cerr << "[GTK4Application] This indicates GTK4 main loop is hanging" << std::endl;
                        std::cerr << "[GTK4Application] Possible causes: missing display, X11/Wayland issues, or GTK4 configuration problems" << std::endl;
                    } else if (status == std::future_status::ready) {
                        try {
                            int result = run_future.get();
                            std::cout << "[GTK4Application] GTK4 main loop completed with result: " << result << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "[GTK4Application] GTK4 main loop failed: " << e.what() << std::endl;
                        }
                    }
                    
                } catch (const std::exception& e) {
                    std::cerr << "[GTK4Application] Error during async GTK4 main loop: " << e.what() << std::endl;
                    // Fallback to synchronous execution
                    std::cout << "[GTK4Application] Falling back to synchronous GTK4 main loop..." << std::endl;
                    
                    int result = g_application_run(G_APPLICATION(gtkApp_), 0, nullptr);
                    std::cout << "[GTK4Application] Synchronous GTK4 main loop completed with result: " << result << std::endl;
                }
                
#else
                std::cout << "[GTK4Application] GTK4 not available (HAVE_GTK4 not defined)" << std::endl;
                std::cout << "[GTK4Application] This is expected if GTK4 development libraries are not installed" << std::endl;
#endif
                
                std::cout << "[GTK4Application] run() method completed" << std::endl;
            }
            
            void GTK4Application::quit() {
                std::cout << "[GTK4Application] Quitting application..." << std::endl;
                cleanup();
            }
            
            void GTK4Application::cleanup() {
                if (gtkApp_) {
#ifdef HAVE_GTK4
                    g_object_unref(gtkApp_);
#endif
                    gtkApp_ = nullptr;
                }
                mainWindow_ = nullptr;
                initialized_ = false;
            }
            
            void GTK4Application::onActivate(GtkApplication* app, gpointer userData) {
                auto* appInstance = static_cast<GTK4Application*>(userData);
                if (appInstance) {
                    appInstance->createMainWindow();
#ifdef HAVE_GTK4
                    if (appInstance->mainWindow_) {
                        gtk_window_present(appInstance->mainWindow_);
                    }
#endif
                }
            }
            
            void GTK4Application::onShutdown(GtkApplication* app, gpointer userData) {
                auto* appInstance = static_cast<GTK4Application*>(userData);
                if (appInstance) {
                    appInstance->cleanup();
                }
            }
            
            // Factory functions
            std::shared_ptr<GTK4TextWidget> createGTK4Text(const std::string& text) {
                return std::make_shared<GTK4TextWidget>(text);
            }
            
            std::shared_ptr<GTK4ButtonWidget> createGTK4Button(const std::string& title, UICallback callback) {
                return std::make_shared<GTK4ButtonWidget>(title, callback);
            }
            
            std::shared_ptr<GTK4VStackWidget> createGTK4VStack(double spacing) {
                return std::make_shared<GTK4VStackWidget>(spacing);
            }
            
            std::shared_ptr<GTK4HStackWidget> createGTK4HStack(double spacing) {
                return std::make_shared<GTK4HStackWidget>(spacing);
            }
            
            // Utility functions
            bool isGTK4Available() {
#ifdef HAVE_GTK4
                return true;
#else
                return false;
#endif
            }
            
            std::string getGTK4Version() {
#ifdef HAVE_GTK4
                return "GTK4 Available";
#else
                return "GTK4 Not Available (Mock Mode)";
#endif
            }
            
            void applyColorToWidget(GtkWidget* widget, const Color& color, const std::string& cssClass) {
                // CSS styling implementation would go here
                std::cout << "[GTK4] Applying color: " << colorToCSSString(color) << std::endl;
            }
            
            std::string colorToCSSString(const Color& color) {
                return "rgba(" + std::to_string(static_cast<int>(color.red * 255)) + ", " +
                       std::to_string(static_cast<int>(color.green * 255)) + ", " +
                       std::to_string(static_cast<int>(color.blue * 255)) + ", " +
                       std::to_string(color.alpha) + ")";
            }
            
        } // namespace GTK4
    } // namespace UI
} // namespace MiniSwift