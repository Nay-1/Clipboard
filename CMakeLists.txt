cmake_minimum_required(VERSION 3.16)
project(ClipboardHistory VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Gui)

set(SOURCES
    src/main.cpp
    src/core/DatabaseManager.cpp
    src/core/ClipboardMonitor.cpp
    src/ui/MainWindow.cpp
    src/ui/HistoryListModel.cpp
    src/ui/HistoryListDelegate.cpp
    src/ui/ImagePreviewWidget.cpp
    src/utils/HotkeyManager.cpp
)

set(HEADERS
    src/core/ClipboardItem.h
    src/core/DatabaseManager.h
    src/core/ClipboardMonitor.h
    src/ui/MainWindow.h
    src/ui/HistoryListModel.h
    src/ui/HistoryListDelegate.h
    src/ui/ImagePreviewWidget.h
    src/utils/HotkeyManager.h
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Gui
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
