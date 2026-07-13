cmake_minimum_required(VERSION 3.19)
project(GatewayClient LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network Charts Sql)

qt_standard_project_setup()

qt_add_executable(GatewayClient
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    tcpclient.h tcpclient.cpp
    logindialog.h logindialog.cpp
    logindialog.ui
)

target_link_libraries(GatewayClient
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        Qt::Charts
        Qt::Sql
)

install(TARGETS GatewayClient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET GatewayClient
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})