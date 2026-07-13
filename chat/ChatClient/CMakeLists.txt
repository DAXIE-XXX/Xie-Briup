cmake_minimum_required(VERSION 3.19)
project(ChatClient LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network Mqtt Sql)
include_directories(net)
include_directories(config)
include_directories(mqtt)
include_directories(db)
qt_standard_project_setup()

qt_add_executable(ChatClient
    WIN32 MACOSX_BUNDLE
    main.cpp
    loginpage.cpp
    loginpage.h
    loginpage.ui
    config/configcontext.cpp config/configcontext.h
    net/netcontext.cpp net/netcontext.h
    mainpage.h mainpage.cpp mainpage.ui
    edituserinfopage.h edituserinfopage.cpp edituserinfopage.ui
    finduserpage.h finduserpage.cpp finduserpage.ui
    friendreqlistpage.h friendreqlistpage.cpp friendreqlistpage.ui
    userchangepasswordpage.h userchangepasswordpage.cpp userchangepasswordpage.ui
    mqtt/mqttcontext.cpp mqtt/mqttcontext.h mqtt/mqttclientmanager.h mqtt/mqttclientmanager.cpp
    db/chathistorymanager.cpp db/chathistorymanager.h
)

target_link_libraries(ChatClient
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        Qt::Mqtt
        Qt::Sql
)

install(TARGETS ChatClient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ChatClient
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
