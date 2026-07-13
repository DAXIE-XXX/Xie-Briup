cmake_minimum_required(VERSION 3.16)

project(SSClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network)

include_directories(data)
include_directories(net)
include_directories(config)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appSSClient
    main.cpp
)

qt_add_qml_module(appSSClient
    URI SSClient
    QML_FILES
        Main.qml
        QML_FILES Map.qml
        RESOURCES
            images/RoadMap.jpg
            images/car.png
            images/its.ico
            images/briup.png
            images/connect.png
            images/control.png
            images/home.png
            images/setting.png
        SOURCES data/datacontext.h data/datacontext.cpp
        SOURCES net/netcontext.h net/netcontext.cpp
        QML_FILES CrossingSet.qml
        QML_FILES CarSet.qml
        SOURCES systemcontrol.h systemcontrol.cpp
        SOURCES config/configcontext.cpp config/configcontext.h
        RESOURCES images/briup.png images/connect.png images/control.png images/home.png images/setting.png
        RESOURCES images/its.ico
        QML_FILES SystemSet.qml
)

qt_add_resources(appSSClient "app_icon"
    PREFIX "/"
    FILES images/its.ico
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSSClient PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSSClient
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSSClient
    PRIVATE
        Qt6::Quick
        Qt6::Network

)

install(TARGETS appSSClient
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
