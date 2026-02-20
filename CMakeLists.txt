cmake_minimum_required(VERSION 3.5)

project(CoreBoard VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools Svg)

set(TS_FILES translations/CoreBoard_en_US.ts translations/CoreBoard_fr_FR.ts translations/CoreBoard_de_DE.ts)

set(TRANSLATABLE_SOURCES
    mainwindow.ui
    labelsettingsdialog.cpp
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
        labelsettingsdialog.h labelsettingsdialog.cpp
        dialogstyle.h dialogstyle.cpp
        ${TRANSLATABLE_SOURCES}
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)

qt_add_executable(CoreBoard
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        CoreBoard.rc
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
            CoreBoard.rc
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TRANSLATABLE_SOURCES} ${TS_FILES})
endif()


target_link_libraries(CoreBoard PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg)

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

# Windows: bundle compiler runtime so install/installer runs on other PCs
if(WIN32)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    include(InstallRequiredSystemLibraries)
    # MinGW: windeployqt and CPack do not copy MinGW runtime DLLs; install them explicitly
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        get_filename_component(_mingw_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
        set(_mingw_runtime_dlls
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll
        )
        foreach(_dll ${_mingw_runtime_dlls})
            set(_path "${_mingw_bin}/${_dll}")
            if(EXISTS "${_path}")
                install(FILES "${_path}" DESTINATION ${CMAKE_INSTALL_BINDIR})
            else()
                message(WARNING "MinGW runtime DLL not found: ${_path} (installer may fail on other PCs)")
            endif()
        endforeach()
    endif()
endif()

install(TARGETS CoreBoard
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Deploy Qt DLLs and plugins so the installed app (and installer) are self-contained
if(WIN32 AND NOT ANDROID)
    if(QT_VERSION_MAJOR GREATER_EQUAL 6)
        # Qt 6.3+: generated script runs windeployqt at install time
        qt_generate_deploy_app_script(
            TARGET CoreBoard
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
            NO_COMPILER_RUNTIME  # we use InstallRequiredSystemLibraries
        )
        install(SCRIPT ${deploy_script})
    else()
        # Qt 5: run windeployqt at install time
        set(_qt_bin_dir "")
        if(TARGET Qt${QT_VERSION_MAJOR}::qmake)
            get_target_property(_qmake_executable Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
            if(_qmake_executable)
                get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
            endif()
        endif()
        if(NOT _qt_bin_dir)
            get_target_property(_widgets_loc Qt${QT_VERSION_MAJOR}::Widgets IMPORTED_LOCATION)
            if(_widgets_loc)
                get_filename_component(_qt_bin_dir "${_widgets_loc}" DIRECTORY)
            endif()
        endif()
        if(_qt_bin_dir)
            find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
            if(WINDEPLOYQT_EXECUTABLE)
                install(CODE "
                    set(_exe \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/CoreBoard.exe\")
                    if(NOT EXISTS \"\${_exe}\")
                        message(FATAL_ERROR \"Expected exe at \${_exe}\")
                    endif()
                    execute_process(
                        COMMAND \"${CMAKE_COMMAND}\" -E env PATH=\"${_qt_bin_dir}\" \"${WINDEPLOYQT_EXECUTABLE}\"
                            --no-compiler-runtime
                            \"\${_exe}\"
                        RESULT_VARIABLE _res
                    )
                    if(_res)
                        message(FATAL_ERROR \"windeployqt failed: \${_res}\")
                    endif()
                ")
            endif()
        endif()
    endif()
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CoreBoard)
endif()

# --- CPack: Windows installer (NSIS) and ZIP
set(CPACK_PACKAGE_NAME "coreBoard")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "coreBoard")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "coreBoard")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_cpack_arch "win64")
else()
    set(_cpack_arch "win32")
endif()
set(CPACK_PACKAGE_FILE_NAME "coreBoard-${_cpack_arch}")
set(CPACK_PACKAGE_EXECUTABLES "CoreBoard" "coreBoard")
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_PACKAGE_NAME "coreBoard")
    set(CPACK_NSIS_DISPLAY_NAME "coreBoard")
    set(CPACK_NSIS_INSTALL_ROOT "\\$PROGRAMFILES64")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\CoreBoard.exe")
    set(CPACK_NSIS_WELCOME_TITLE "Welcome to coreBoard Setup")
    set(CPACK_NSIS_FINISH_TITLE "coreBoard installed successfully")
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "\$INSTDIR\\\\bin\\\\CoreBoard.exe")
    set(CPACK_NSIS_BRANDING_TEXT "coreBoard")
    set(CPACK_NSIS_IGNORE_LICENSE_PAGE ON)
    set(_installer_dir "${CMAKE_CURRENT_SOURCE_DIR}/installer")
    if(EXISTS "${_installer_dir}/coreBoard.ico")
        set(CPACK_NSIS_MUI_ICON "${_installer_dir}/coreBoard.ico")
        set(CPACK_NSIS_MUI_UNIICON "${_installer_dir}/coreBoard.ico")
    endif()
    if(EXISTS "${_installer_dir}/welcome.bmp")
        set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${_installer_dir}/welcome.bmp")
    endif()
    if(EXISTS "${_installer_dir}/header.bmp")
        set(CPACK_NSIS_MUI_HEADERIMAGE "${_installer_dir}/header.bmp")
    endif()
    set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\\\coreBoard.lnk' '$INSTDIR\\\\bin\\\\CoreBoard.exe' ''")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$DESKTOP\\\\coreBoard.lnk'")
else()
    set(CPACK_GENERATOR "ZIP")
endif()
include(CPack)
