cmake_minimum_required(VERSION 3.21)

project(pubg_rangefinder VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/app/main.cpp
    src/app/AppController.cpp
    src/platform/win/GlobalHotkeyManager.cpp
    src/platform/win/TrayController.cpp
    src/overlay/OverlayWindow.cpp
    src/ui/MainWindow.cpp
    src/ui/KeyCaptureLineEdit.cpp
)

set(PROJECT_HEADERS
    src/app/AppController.h
    src/config/UiSettingsDraft.h
    src/platform/win/GlobalHotkeyManager.h
    src/platform/win/TrayController.h
    src/overlay/OverlayWindow.h
    src/ui/MainWindow.h
    src/ui/KeyCaptureLineEdit.h
)

qt_add_executable(pubg_rangefinder
    WIN32
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_include_directories(pubg_rangefinder PRIVATE src)

target_link_libraries(pubg_rangefinder PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

if(WIN32)
    target_link_libraries(pubg_rangefinder PRIVATE user32 dwmapi)
endif()

set_target_properties(pubg_rangefinder PROPERTIES
    OUTPUT_NAME "pubg_rangefinder"
)
