程序在GTK4初始化时卡住了，这可能是因为GTK4库没有正确安装或配置。让我检查完整的错误输出。

程序在GTK4初始化时停止了，这表明GTK4库可能没有正确安装。让我查看GTK4Backend的实现，看看初始化过程中可能出现的问题。

src\ui\GTK4Backend.cpp 1 - 50

我看到代码中有HAVE_GTK4的条件编译，如果没有GTK4库，会使用mock函数。让我查看GTK4Application的initialize方法，看看初始化过程。

src\ui\GTK4Backend.cpp 337 - 370

我看到问题了，程序在GTK4初始化后没有继续执行。让我查看isGTK4Available函数和run方法，看看是否有问题。

src\ui\GTK4Backend.cpp 386 - 420

我看到问题了！在run方法中，如果gtkApp_为nullptr（因为HAVE_GTK4未定义），程序会直接返回而不执行任何UI逻辑。让我查看isGTK4Available函数的实现。

src\ui\GTK4Backend.cpp 451 - 481

现在我明白了问题所在。系统中没有安装GTK4库，所以HAVE_GTK4宏未定义，导致程序运行在模拟模式（mock mode）。在这种模式下，GTK4Application::run()方法会直接返回，不会显示任何UI窗口。让我查看UIIntegration.cpp中的run方法，看看它是如何调用GTK4Application的。

src\ui\UIIntegration.cpp 1 - 50

让我查看runUIApplication方法的实现，看看它是如何处理不同后端的。

src\ui\UIIntegration.cpp 183 - 220

让我查看selectBestBackend方法，看看它是如何选择后端的。

src\ui\UIIntegration.cpp 281 - 295

现在让我查看isBackendAvailable方法，看看它是如何检查GTK4是否可用的。

src\ui\UIIntegration.cpp 224 - 240

现在我明白了整个问题的根源。让我查看Mock后端的UIApplication实现，看看它是如何工作的。

src\ui\UIRuntime.cpp 1 - 50

让我查看UIApplication的run方法实现，看看Mock后端是如何处理UI应用运行的。