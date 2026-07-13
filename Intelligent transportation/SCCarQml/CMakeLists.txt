cmake_minimum_required(VERSION 3.16)

project(SCCar VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network)
include_directories(data)
include_directories(net)
include_directories(config)
qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appSCCar
    main.cpp
)

qt_add_qml_module(appSCCar
    URI SCCar
    QML_FILES Main.qml
    RESOURCES images/car.png
    SOURCES data/datacontext.cpp data/datacontext.h
    SOURCES net/netcontext.cpp net/netcontext.h
    SOURCES systemcontrol.h systemcontrol.cpp
    SOURCES config/configcontext.cpp config/configcontext.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSCCar PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSCCar
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSCCar
    PRIVATE
        Qt6::Quick
        Qt6::Network
)

install(TARGETS appSCCar
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
