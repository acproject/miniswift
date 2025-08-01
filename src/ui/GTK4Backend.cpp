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
#define gtk_widget_set_visible(widget, visible)
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
                    gtk_widget_set_visible(nativeWidget_, TRUE);
#else
                    std::cout << "[GTK4Widget] Show widget (mock)" << std::endl;
#endif
                }
            }
            
            void GTK4Widget::hide() {
                if (nativeWidget_) {
#ifdef HAVE_GTK4
                    gtk_widget_set_visible(nativeWidget_, FALSE);
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
                std::cout << "[DEBUG] GTK4TextWidget constructor called with text: " << text << std::endl;
#ifdef HAVE_GTK4
                // Check if we're in the main thread
                std::cout << "[DEBUG] Current thread ID: " << std::this_thread::get_id() << std::endl;
                
                // Don't create GTK widget immediately - wait for GTK4 to be fully initialized
                // The widget will be created in render() when GTK4 is ready
                label_ = nullptr;
                std::cout << "[GTK4TextWidget] Created text widget: \"" << text << "\" (deferred GTK4 creation)" << std::endl;
#else
                std::cout << "[DEBUG] Using mock mode (HAVE_GTK4 not defined)" << std::endl;
                label_ = nullptr;
                std::cout << "[GTK4TextWidget] Created text widget: \"" << text << "\" (mock)" << std::endl;
#endif
            }
            
            void GTK4TextWidget::render() {
#ifdef HAVE_GTK4
                // Create GTK widget if not already created
                if (!label_ && !getNativeWidget()) {
                    std::cout << "[GTK4TextWidget] Creating GTK4 label widget" << std::endl;
                    label_ = GTK_LABEL(gtk_label_new(getText().c_str()));
                    if (label_) {
                        setNativeWidget(GTK_WIDGET(label_));
                        std::cout << "[GTK4TextWidget] Successfully created GTK4 label widget" << std::endl;
                    } else {
                        std::cout << "[GTK4TextWidget] Failed to create GTK4 label widget" << std::endl;
                        return;
                    }
                }
#endif
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
                std::cout << "[DEBUG] GTK4ButtonWidget constructor called with title: " << title << std::endl;
#ifdef HAVE_GTK4
                // Check if we're in the main thread
                std::cout << "[DEBUG] Current thread ID: " << std::this_thread::get_id() << std::endl;
                
                // Don't create GTK widget immediately - wait for GTK4 to be fully initialized
                // The widget will be created in render() when GTK4 is ready
                button_ = nullptr;
                std::cout << "[GTK4ButtonWidget] Created button: \"" << title << "\" (deferred GTK4 creation)" << std::endl;
#else
                std::cout << "[DEBUG] Using mock mode (HAVE_GTK4 not defined)" << std::endl;
                button_ = nullptr;
                std::cout << "[GTK4ButtonWidget] Created button: \"" << title << "\" (mock)" << std::endl;
#endif
            }
            
            void GTK4ButtonWidget::render() {
#ifdef HAVE_GTK4
                // Create GTK widget if not already created
                if (!button_ && !getNativeWidget()) {
                    std::cout << "[GTK4ButtonWidget] Creating GTK4 button widget" << std::endl;
                    button_ = GTK_BUTTON(gtk_button_new_with_label(getTitle().c_str()));
                    if (button_) {
                        setNativeWidget(GTK_WIDGET(button_));
                        connectSignals();
                        std::cout << "[GTK4ButtonWidget] Successfully created GTK4 button widget" << std::endl;
                    } else {
                        std::cout << "[GTK4ButtonWidget] Failed to create GTK4 button widget" << std::endl;
                        return;
                    }
                }
#endif
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
                std::cout << "[DEBUG] GTK4VStackWidget constructor called with spacing: " << spacing << std::endl;
                std::cout << "[DEBUG] Current thread ID: " << std::this_thread::get_id() << std::endl;
#ifdef HAVE_GTK4
                // Defer GTK widget creation to render() method
                std::cout << "[DEBUG] Deferring GTK4 vertical box widget creation" << std::endl;
                box_ = nullptr;
                std::cout << "[GTK4VStackWidget] Created VStack with spacing: " << spacing << " (deferred creation)" << std::endl;
#else
                std::cout << "[DEBUG] Using mock mode (HAVE_GTK4 not defined)" << std::endl;
                box_ = nullptr;
                std::cout << "[GTK4VStackWidget] Created VStack with spacing: " << spacing << " (mock)" << std::endl;
#endif
            }
            
            void GTK4VStackWidget::render() {
#ifdef HAVE_GTK4
                // Create GTK widget if not already created
                if (!box_ && !getNativeWidget()) {
                    std::cout << "[GTK4VStackWidget] Creating GTK4 vertical box widget" << std::endl;
                    box_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, static_cast<int>(getSpacing())));
                    if (box_) {
                        setNativeWidget(GTK_WIDGET(box_));
                        std::cout << "[GTK4VStackWidget] Successfully created GTK4 vertical box widget" << std::endl;
                    } else {
                        std::cout << "[GTK4VStackWidget] Failed to create GTK4 vertical box widget" << std::endl;
                        return;
                    }
                }
#endif
                updateGTKSpacing();
                
                // Add children to GTK box
                for (auto& child : getChildren()) {
                    if (child) {
                        child->render();  // Render child first to create its native widget
                        addGTKChild(child);  // Then add it to the parent
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
                if (!box_ || !child) {
                    return;
                }
                
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
                std::cout << "[DEBUG] GTK4HStackWidget constructor called with spacing: " << spacing << std::endl;
#ifdef HAVE_GTK4
                // Check if we're in the main thread
                std::cout << "[DEBUG] Current thread ID: " << std::this_thread::get_id() << std::endl;
                
                // Don't create GTK widget immediately - wait for GTK4 to be fully initialized
                // The widget will be created in render() when GTK4 is ready
                box_ = nullptr;
                std::cout << "[GTK4HStackWidget] Created HStack with spacing: " << spacing << " (deferred GTK4 creation)" << std::endl;
#else
                std::cout << "[DEBUG] Using mock mode (HAVE_GTK4 not defined)" << std::endl;
                box_ = nullptr;
                std::cout << "[GTK4HStackWidget] Created HStack with spacing: " << spacing << " (mock)" << std::endl;
#endif
            }
            
            void GTK4HStackWidget::render() {
#ifdef HAVE_GTK4
                // Create GTK widget if not already created
                if (!box_ && !getNativeWidget()) {
                    std::cout << "[GTK4HStackWidget] Creating GTK4 horizontal box widget" << std::endl;
                    box_ = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, static_cast<int>(getSpacing())));
                    if (box_) {
                        setNativeWidget(GTK_WIDGET(box_));
                        std::cout << "[GTK4HStackWidget] Successfully created GTK4 horizontal box widget" << std::endl;
                    } else {
                        std::cout << "[GTK4HStackWidget] Failed to create GTK4 horizontal box widget" << std::endl;
                        return;
                    }
                }
#endif
                updateGTKSpacing();
                
                // Add children to GTK box
                for (auto& child : getChildren()) {
                    if (child) {
                        child->render();  // Render child first to create its native widget
                        addGTKChild(child);  // Then add it to the parent
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
                if (!box_ || !child) {
                    return;
                }
                
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
                if (!gtkApp_) {
                    std::cout << "[GTK4Application] Cannot create main window: gtkApp_ is null" << std::endl;
                    return;
                }
                
                if (mainWindow_) {
                    std::cout << "[GTK4Application] Main window already exists" << std::endl;
                    return;
                }
                
#ifdef HAVE_GTK4
                std::cout << "[GTK4Application] Creating GTK application window..." << std::endl;
                mainWindow_ = GTK_WINDOW(gtk_application_window_new(gtkApp_));
                gtk_window_set_title(mainWindow_, "MiniSwift UI");
                gtk_window_set_default_size(mainWindow_, 800, 600);
                std::cout << "[GTK4Application] Main window created successfully" << std::endl;
#else
                std::cout << "[GTK4Application] Created main window (mock)" << std::endl;
#endif
            }
            
            void GTK4Application::setMainWindowContent(std::shared_ptr<UIWidget> content) {
                if (!content) {
                    std::cout << "[GTK4Application] Warning: content is null" << std::endl;
                    return;
                }
                
                // If GTK is not running yet, store content for later
                if (!isRunning_) {
                    std::cout << "[GTK4Application] GTK not running yet, storing content for later" << std::endl;
                    pendingContent_ = content;
                    return;
                }
                
                // Ensure main window exists
                if (!mainWindow_) {
                    std::cout << "[GTK4Application] Creating main window for content..." << std::endl;
                    createMainWindow();
                }
                
                if (!mainWindow_) {
                    std::cout << "[GTK4Application] Error: Failed to create main window" << std::endl;
                    return;
                }
                
                // Render the content to ensure GTK widgets are created
                std::cout << "[GTK4Application] Rendering content widget" << std::endl;
                content->render();
                
                auto gtk4Content = std::dynamic_pointer_cast<GTK4Widget>(content);
                if (gtk4Content && gtk4Content->getNativeWidget()) {
#ifdef HAVE_GTK4
                    gtk_window_set_child(mainWindow_, gtk4Content->getNativeWidget());
                    // Show the window after setting content
                    gtk_window_present(mainWindow_);
#endif
                    std::cout << "[GTK4Application] Set main window content and presented window" << std::endl;
                } else {
                    std::cout << "[GTK4Application] Warning: content is not a valid GTK4Widget, creating placeholder" << std::endl;
                    // Create a simple placeholder widget for mock content
#ifdef HAVE_GTK4
                    GtkWidget* placeholder = gtk_label_new("MiniSwift UI Application\n(Mock Mode - GTK4 widgets simulated)");
                    if (placeholder) {
                        gtk_window_set_child(mainWindow_, placeholder);
                        gtk_window_present(mainWindow_);
                        std::cout << "[GTK4Application] Set placeholder content and presented window" << std::endl;
                    }
#endif
                }
            }
            
            void GTK4Application::run() {
                std::cout << "[GTK4Application] Starting run() method..." << std::endl;
                std::cout << "[GTK4Application] Thread ID: " << std::this_thread::get_id() << std::endl;
                
                if (!gtkApp_) {
                    std::cout << "[GTK4Application] Running in mock mode (gtkApp_ is nullptr)" << std::endl;
                    std::cout << "[GTK4Application] This means GTK4 is not available or initialization failed" << std::endl;
                    std::cout << "[GTK4Application] Simulating UI application running..." << std::endl;
                    
                    // In mock mode, simulate the application running for a short time
                    std::cout << "[GTK4Application] Mock application will run for 3 seconds and then exit" << std::endl;
                    isRunning_ = true;
                    
                    // Simulate application running for 3 seconds instead of infinite loop
                    for (int i = 0; i < 30; i++) {
                        if (!isRunning_) break; // Allow early exit
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    
                    isRunning_ = false;
                    std::cout << "[GTK4Application] Mock application completed" << std::endl;
                    return;
                }
                
#ifdef HAVE_GTK4
                std::cout << "[GTK4Application] GTK4 is available, running main loop..." << std::endl;
                
                std::cout << "[GTK4Application] About to call g_application_run()..." << std::endl;
                
                // Set running flag before starting GTK main loop
                isRunning_ = true;
                
                // Run GTK main loop in the main thread (blocking)
                int result = g_application_run(G_APPLICATION(gtkApp_), 0, nullptr);
                std::cout << "[GTK4Application] GTK main loop completed with result: " << result << std::endl;
                
                isRunning_ = false;
                std::cout << "[GTK4Application] GTK application exiting..." << std::endl;
                
#else
                std::cout << "[GTK4Application] GTK4 not available (HAVE_GTK4 not defined)" << std::endl;
                std::cout << "[GTK4Application] This is expected if GTK4 development libraries are not installed" << std::endl;
                std::cout << "[GTK4Application] Simulating UI application running..." << std::endl;
                
                // In mock mode, simulate the application running for a short time
                std::cout << "[GTK4Application] Mock application will run for 3 seconds and then exit" << std::endl;
                isRunning_ = true;
                
                // Simulate application running for 3 seconds instead of infinite loop
                for (int i = 0; i < 30; i++) {
                    if (!isRunning_) break; // Allow early exit
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                
                isRunning_ = false;
                std::cout << "[GTK4Application] Mock application completed" << std::endl;
#endif
                
                std::cout << "[GTK4Application] run() method completed" << std::endl;
            }
            
            void GTK4Application::quit() {
                std::cout << "[GTK4Application] Quitting application..." << std::endl;
                
                // Quit GTK application if running in daemon thread
                if (isRunning_ && gtkApp_) {
#ifdef HAVE_GTK4
                    std::cout << "[GTK4Application] Requesting GTK application quit..." << std::endl;
                    g_application_quit(G_APPLICATION(gtkApp_));
#endif
                }
                
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
                    std::cout << "[GTK4Application] onActivate called - GTK is now fully initialized" << std::endl;
                    // Only create main window if it doesn't exist
                    if (!appInstance->mainWindow_) {
                        appInstance->createMainWindow();
                        std::cout << "[GTK4Application] Main window created in onActivate" << std::endl;
                    } else {
                        std::cout << "[GTK4Application] Main window already exists in onActivate" << std::endl;
                    }
                    
                    // If we have pending content, set it now that GTK is ready
                    if (appInstance->pendingContent_) {
                        std::cout << "[GTK4Application] Setting pending content now that GTK is ready" << std::endl;
                        appInstance->setMainWindowContent(appInstance->pendingContent_);
                        appInstance->pendingContent_.reset();
                    } else {
                        // Create a simple test widget for demonstration
                        std::cout << "[GTK4Application] Creating test content for demonstration" << std::endl;
                        auto testText = createGTK4Text("Hello GTK4 World! This is a test.");
                        if (testText) {
                            appInstance->setMainWindowContent(testText);
                            std::cout << "[GTK4Application] Test content set successfully" << std::endl;
                        }
                    }
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
                std::cout << "[DEBUG] createGTK4Text called with text: " << text << std::endl;
                std::cout << "[DEBUG] Creating GTK4TextWidget..." << std::endl;
                auto widget = std::make_shared<GTK4TextWidget>(text);
                std::cout << "[DEBUG] GTK4TextWidget created successfully" << std::endl;
                return widget;
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