cmake_minimum_required( VERSION 2.6 )

set(PROJECT razor-panel )

set(QT_USE_QTXML 1)
set(QT_USE_QTDBUS 1)


set(razor-panel_PRIV_H_FILES
    razorpanel.h
    razorpanelapplication.h
    razorpanellayout.h
    razorpanelpluginconfigdialog.h
    config/configpaneldialog.h
    plugin.h
    razorpanellimits.h
    popupmenu.h
    pluginmoveprocessor.h
)

set(razor-panel_PUB_H_FILES
    irazorpanelplugin.h
    irazorpanel.h
)

set(razor-panel_CPP_FILES
    main.cpp
    razorpanel.cpp
    razorpanelapplication.cpp
    razorpanellayout.cpp
    razorpanelpluginconfigdialog.cpp
    config/configpaneldialog.cpp
    plugin.cpp
    popupmenu.cpp
    pluginmoveprocessor.cpp
)

set(MOCS
    razorpanel.h
    razorpanelapplication.h
    razorpanellayout.h
    razorpanelpluginconfigdialog.h
    config/configpaneldialog.h
    plugin.h
    pluginmoveprocessor.h
)

set(LIBRARIES
    razorqt
    qtxdg
)

# QRC files .................
set(RESOURCES

)

set (razor-panel_UI_FILES
    config/configpaneldialog.ui
)

QT4_WRAP_UI(UI_HEADERS ${razor-panel_UI_FILES})

add_definitions(-DDEFULT_PANEL_PLUGINS=\"MainMenu,Quicklaunch,TaskBar,Tray,Clock\")

include(RazorTranslate)

file (GLOB CONFIG_FILES     resources/*.conf)

############################################

if (NOT CMAKE_BUILD_TYPE)
    set ( CMAKE_BUILD_TYPE Release )
endif (NOT CMAKE_BUILD_TYPE)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions (-Wall)
#    add_definitions (-Wall -pedantic)
endif()


set(PLUGIN_DESKTOPS_DIR
    ${CMAKE_INSTALL_PREFIX}/share/razor/${PROJECT}
)

add_definitions(-DPLUGIN_DESKTOPS_DIR=\"${PLUGIN_DESKTOPS_DIR}\")


#*******************************************
project(${PROJECT})

find_package(Qt4 REQUIRED)
#find_package ( X11 REQUIRED )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_SOURCES ${MOCS})
qt4_wrap_ui(UI_HEADERS ${FORMS})
qt4_add_resources(QRC_SOURCES ${RESOURCES})

# Translations **********************************
include(RazorTranslate)
razor_translate_ts(razor-runner_QM_FILES 
    SOURCES
        ${razor-panel_PUB_H_FILES}
        ${razor-panel_PRIV_H_FILES}
        ${razor-panel_CPP_FILES} 
        ${razor-panel_UI_FILES}
)
#************************************************


set (PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/razor-panel/)
add_definitions(-DPLUGIN_DIR=\"${PLUGIN_DIR}\")
message(STATUS "Panel plugins location: ${PLUGIN_DIR}")

add_executable(${PROJECT} ${razor-panel_PUB_H_FILES} ${razor-panel_PRIV_H_FILES} ${razor-panel_CPP_FILES} ${MOC_SOURCES} ${razor-runner_QM_FILES} ${QRC_SOURCES} ${UI_HEADERS})
target_link_libraries(${PROJECT} ${LIBRARIES} ${QT_LIBRARIES})

install(TARGETS ${PROJECT} RUNTIME DESTINATION bin)
install(FILES   ${CONFIG_FILES}    DESTINATION ${RAZOR_ETC_XDG_DIRECTORY}/razor/)
install(FILES  ${razor-panel_PUB_H_FILES} DESTINATION include/razorqt)
