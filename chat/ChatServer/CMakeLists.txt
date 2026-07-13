cmake_minimum_required(VERSION 3.19)
project(ChatServer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network Sql Mqtt)
include_directories(net)
include_directories(log)
include_directories(config)
include_directories(db)
include_directories(mqtt)
qt_standard_project_setup()

qt_add_executable(ChatServer
    main.cpp
    net/mytcpserver.h net/mytcpserver.cpp
    net/mytcpsocket.h net/mytcpsocket.cpp
    log/logcontext.h log/logcontext.cpp
    net/netcontext.h net/netcontext.cpp
    systemcontrol.h systemcontrol.cpp
    config/configcontext.h config/configcontext.cpp
    net/threadshandle.h net/threadshandle.cpp
    db/dbcontext.h db/dbcontext.cpp
    db/dbexec.h db/dbexec.cpp
    mqtt/mqttclientmanager.h mqtt/mqttclientmanager.cpp
    mqtt/mqttcontext.h mqtt/mqttcontext.cpp
)

target_link_libraries(ChatServer
    PRIVATE
        Qt::Core
        Qt::Network
        Qt::Sql
        Qt::Mqtt
)

install(TARGETS ChatServer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
