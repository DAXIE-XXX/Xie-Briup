cmake_minimum_required(VERSION 3.19)
project(SHServer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network Sql)
include_directories(net)
include_directories(log)
include_directories(config)
include_directories(db)
qt_standard_project_setup()

qt_add_executable(SHServer
    main.cpp
    net/mytcpserver.h
    net/mytcpserver.cpp
    net/mytcpsocket.h
    net/mytcpsocket.cpp
    net/threshold.h
    log/logcontext.h
    log/logcontext.cpp
    net/netcontext.h
    net/netcontext.cpp
    systemcontrol.h
    systemcontrol.cpp
    config/configcontext.h
    config/configcontext.cpp
    net/threadshandle.h
    net/threadshandle.cpp
    db/dbcontext.h
    db/dbcontext.cpp
    db/dbexec.h
    db/dbexec.cpp
)

target_link_libraries(SHServer
    PRIVATE
        Qt::Core
        Qt::Network
        Qt::Sql
)

install(TARGETS SHServer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)