# CMakeLists.txt for ClamAV-GUI
# This file is used to build the ClamAV-GUI application using CMake.
# It sets up the project, finds the required Qt components, and configures the build process.
# It also handles translation files and UI files, ensuring they are processed correctly.
# The project is set to use Qt6 by default, but can be changed to Qt5 by modifying the QT_VERSION variable.
# The translations are managed using the Qt Linguist tools, and the UI files are processed to generate header files.
# The final executable is named clamav-gui and is installed to the usr/bin directory.
# The CMake version required is 3.21 or higher. cmake version is set to 3.21 or higher to ensure compatibility with the features used in this project.
# The project includes the necessary Qt components for GUI applications, networking, and core functionality.
cmake_minimum_required(VERSION 3.21)
project(ClamAV-GUI
    VERSION 1.1.8
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
message(STATUS "FILE_CPP: ${FILE_CPP}")

if(QT_MAJOR_VERSION EQUAL 5)
    qt5_add_translation(QM_FILES ${FILE_TS})
    qt5_wrap_ui(UI_FILES_H "${FILE_UI}")
    add_custom_target(UpdateTranslations
        COMMAND Qt5::lupdate -locations relative ${FILE_CPP} ${FILE_UI} -ts ${FILE_TS}
        BYPRODUCTS ${FILE_TS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Updating translation files"
        VERBATIM
        )
    
    add_executable(clamav-gui
        ${FILE_TS}
        ${TRANSLATION_QRC_FILE}
        "${UI_FILES_H}"
        "${FILE_CPP}"
        resources.qrc
        )
elseif(QT_MAJOR_VERSION EQUAL 6)
    # Qt6: Generate .qm files during build
    qt_add_translation(QM_FILES ${FILE_TS})
    qt_wrap_ui(UI_FILES_H "${FILE_UI}")
    
    # Create a custom target to ensure .qm files are generated
    add_custom_target(generate_translations ALL
        DEPENDS ${QM_FILES}
        COMMENT "Generating translation files"
        )
    
    add_executable(clamav-gui
        ${FILE_TS}
        ${TRANSLATION_QRC_FILE}
        "${UI_FILES_H}"
        "${FILE_CPP}"
        resources.qrc
        )
    
    # Ensure the executable depends on the generated .qm files
    add_dependencies(clamav-gui generate_translations)
    
    # Copy .qm files to the build directory for runtime access
    foreach(QM_FILE ${QM_FILES})
        get_filename_component(QM_NAME ${QM_FILE} NAME)
        add_custom_command(TARGET clamav-gui POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                ${QM_FILE}
                ${CMAKE_CURRENT_BINARY_DIR}/${QM_NAME}
            COMMENT "Copying ${QM_NAME} to build directory"
            )
    endforeach()
endif()

target_include_directories(clamav-gui
    PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}  # Add this to find generated files
    ${CMAKE_SOURCE_DIR}
    )

target_link_libraries(clamav-gui
    PRIVATE
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Core5Compat
    )

option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    if(DEFINED ENV{GITHUB_ACTIONS} AND "$ENV{GITHUB_ACTIONS}" STREQUAL "true")
        add_subdirectory(test)
        message(STATUS "Running in GitHub Actions")
    else()
        add_subdirectory(qtest)
        message(STATUS "Not running in GitHub Actions")
    endif()
    message(STATUS "Building tests")
    enable_testing()
endif()

include(install.cmake)
include(cpack.cmake)