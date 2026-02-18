cmake_minimum_required(VERSION 3.5)

project(CoreBoard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES translations/CoreBoard_en_US.ts translations/CoreBoard_fr_FR.ts translations/CoreBoard_de_DE.ts)

set(TRANSLATABLE_SOURCES
    mainwindow.ui
    layouteditor.h layouteditor.cpp
    main.cpp
    mainwindow.cpp
    mainwindow.h
    versioninfo.h
    version_info.ui
    keyboardwidget.h keyboardwidget.cpp
    windowskeylistener.h windowskeylistener.cpp
    layouteditorgraphicsview.h layouteditorgraphicsview.cpp
)

set(PROJECT_SOURCES
        icons/redo.png icons/undo.png
        resources.qrc
        resizableellipseitem.h resizableellipseitem.cpp
        resizablepolygonitem.h resizablepolygonitem.cpp
        layoutsettings.h layoutsettings.cpp
        settingsdialog.h settingsdialog.cpp
        ${TRANSLATABLE_SOURCES}
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)

qt_add_executable(CoreBoard
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resizablerectitem.h resizablerectitem.cpp
        dialogtextchange.h dialogtextchange.cpp
        dialogkeycodechange.h
        dialogkeycodechange.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET CoreBoard APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATABLE_SOURCES} ${TS_FILES})
else()
    if(ANDROID)
        add_library(CoreBoard SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(CoreBoard
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATABLE_SOURCES} ${TS_FILES})
endif()


target_link_libraries(CoreBoard PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CoreBoard)
endif()
set_target_properties(CoreBoard PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS CoreBoard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CoreBoard)
endif()
