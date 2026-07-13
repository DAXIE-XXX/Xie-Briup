cmake_minimum_required(VERSION 3.19)
project(SHGateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC 编译器修复：C++17 __cplusplus 宏 + UTF-8 源文件编码
if(MSVC)
    add_compile_options(/Zc:__cplusplus /utf-8 /permissive-)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets SerialPort Network)

qt_standard_project_setup()

qt_add_executable(SHGateway
    main.cpp
    mainpage.cpp
    mainpage.h
    mainpage.ui
    serialcontext.h serialcontext.cpp
    netcontext.h netcontext.cpp
)

target_link_libraries(SHGateway
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::SerialPort
        Qt::Network
)

install(TARGETS SHGateway
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET SHGateway
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
