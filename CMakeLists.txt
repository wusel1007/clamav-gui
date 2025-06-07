# CMakeLists.txt for ClamAV-GUI
# This file is used to build the ClamAV-GUI application using CMake.
# It sets up the project, finds the required Qt components, and configures the build process.
# It also handles translation files and UI files, ensuring they are processed correctly.
# The project is set to use Qt6 by default, but can be changed to Qt5 by modifying the QT_VERSION variable.
# The translations are managed using the Qt Linguist tools, and the UI files are processed to generate header files.
# The final executable is named ClamAV-GUI and is installed to the usr/bin directory.
# The CMake version required is 3.21 or higher. cmake version is set to 3.21 or higher to ensure compatibility with the features used in this project.
# The project includes the necessary Qt components for GUI applications, networking, and core functionality.
cmake_minimum_required(VERSION 3.21)
project(ClamAV-GUI
    VERSION 1.1.4
    DESCRIPTION "ClamAV GUI for Linux"
    HOMEPAGE_URL ""
    LANGUAGES CXX
    )

set(CMAKE_CXX_STANDARD 11)
set(QT_MAJOR_VERSION 6)

# find_package magic provided by the Qt6Config.cmake or Qt5Config.cmake
# This will automatically set the correct Qt version based on the installed Qt version.
# If you want to use Qt5, change QT_VERSION to 5.
find_package(Qt${QT_MAJOR_VERSION} REQUIRED COMPONENTS Widgets Core Network Gui LinguistTools)

if(QT_MAJOR_VERSION EQUAL 6)
    find_package(Qt${QT_MAJOR_VERSION} REQUIRED COMPONENTS Core5Compat)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Set the translation resource file, should not use GLOB but rather a specific file
file(GLOB FILE_TS
    translations/*.ts
    )

file(GLOB FILE_UI
    ui/*.ui
    )

file(GLOB FILE_CPP src/*.cpp src/*.h)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

if(QT_MAJOR_VERSION EQUAL 5)
    qt5_add_translation(QM_FILES ${FILE_TS})
    qt5_wrap_ui(UI_FILES_H "${FILE_UI}")

    add_custom_target(UpdateTranslations
        COMMAND Qt5::lupdate -locations relative "${FILE_CPP}" "${FILE_UI}" -ts "${FILE_TS}"
        BYPRODUCTS ${FILE_TS}
        )
elseif(QT_MAJOR_VERSION EQUAL 6)
    # Qt6 generates the qm files automatically, so we don't need to run lupdate manually.
    qt_add_translation(QM_FILES ${FILE_TS})
    qt_wrap_ui(UI_FILES_H "${FILE_UI}")
endif()

message(STATUS "FILE_CPP: ${FILE_CPP}")

add_executable(ClamAV-GUI
    ${FILE_TS}
    ${TRANSLATION_QRC_FILE}
    "${UI_FILES_H}"
    "${FILE_CPP}"
    resources.qrc
    )
target_include_directories(ClamAV-GUI
    PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    )

target_link_libraries(ClamAV-GUI
    PRIVATE
    Qt${QT_VERSION}::Gui
    Qt${QT_VERSION}::Widgets
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Network
    Qt${QT_VERSION}::Core5Compat
    )

include(install.cmake)

include(cpack.cmake)