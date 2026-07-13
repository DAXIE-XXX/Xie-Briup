cmake_minimum_required(VERSION 3.19)
project(SCServer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network)
include_directories(net)
include_directories(log)
include_directories(config)
qt_standard_project_setup()

qt_add_executable(SCServer
    main.cpp
    net/mytcpserver.h net/mytcpserver.cpp
    net/mytcpsocket.h net/mytcpsocket.cpp
    log/logcontext.h log/logcontext.cpp
    net/netcontext.h net/netcontext.cpp
    systemcontrol.h systemcontrol.cpp
    config/configcontext.h config/configcontext.cpp
)

target_link_libraries(SCServer
    PRIVATE
        Qt::Core
        Qt::Network
)

install(TARGETS SCServer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
