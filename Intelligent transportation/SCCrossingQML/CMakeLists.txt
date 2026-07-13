cmake_minimum_required(VERSION 3.16)

project(SCCrossing VERSION 0.1 LANGUAGES CXX)
include_directories(data)
include_directories(net)
include_directories(config)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appSCCrossing
    main.cpp
)

qt_add_qml_module(appSCCrossing
    URI SCCrossing
    QML_FILES
        Main.qml
        RESOURCES images/RoadSig.jpg
        SOURCES data/datacontext.h data/datacontext.cpp
        SOURCES systemcontrol.h systemcontrol.cpp
        SOURCES net/netcontext.cpp net/netcontext.h
        SOURCES config/configcontext.cpp config/configcontext.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSCCrossing PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSCCrossing
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSCCrossing
    PRIVATE
        Qt6::Quick
        Qt6::Network
)

install(TARGETS appSCCrossing
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
