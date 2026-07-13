cmake_minimum_required(VERSION 3.19)
project(GatewayClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC 编译器修复：C++17 __cplusplus 宏 + UTF-8 源文件编码
if(MSVC)
    add_compile_options(/Zc:__cplusplus /utf-8 /permissive-)
endif()

# Qt 6.5+ 模块：Quick 用于 Android 移动端 UI，保留 Charts 用于图表
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Quick QuickControls2 Network Charts Widgets)

qt_standard_project_setup()

qt_add_executable(GatewayClient
    main.cpp
    # C++ 后端（从桌面版保留，平台无关）
    tcpclient.h tcpclient.cpp
    # 新建：业务逻辑控制器，暴露给 QML
    sensorcontroller.h sensorcontroller.cpp
)

# 显式编译 QML 资源文件到二进制中
qt6_add_resources(GatewayClient "qml_resources"
    PREFIX "/"
    FILES
        qml/main.qml
        qml/LoginPage.qml
        qml/DashboardPage.qml
        qml/ControlPage.qml
        qml/ChartPage.qml
        qml/ThresholdPage.qml
        qml/AlertDialog.qml
)

target_link_libraries(GatewayClient
    PRIVATE
        Qt::Core
        Qt::Quick
        Qt::QuickControls2
        Qt::Network
        Qt::Charts
        Qt::Widgets
)

# 将 QML 插件（QtCharts 等）静态链接到程序中，确保 Android APK 中不会遗漏
qt_import_qml_plugins(GatewayClient)

# ===== Android 平台配置 =====
if(ANDROID)
    # Android 包配置（模板目录含网络权限）
    set_target_properties(GatewayClient PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android"
        QT_ANDROID_PACKAGE_NAME "com.smarthome.gatewayclient"
        QT_ANDROID_VERSION_CODE "1"
        QT_ANDROID_VERSION_NAME "1.0.0"
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 34
    )
else()
    # 桌面端安装配置（开发调试用）
    install(TARGETS GatewayClient
        BUNDLE  DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()
