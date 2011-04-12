add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51014)

add_subdirectory(icon)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
   ${CMAKE_CURRENT_BINARY_DIR}/../libs/internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/service
   ${CMAKE_CURRENT_SOURCE_DIR}/../backends/NetworkManager
   )


set(monolithic_SRCS

   main.cpp

   # base client code
   monolithic.cpp

   # tray icon
   knetworkmanagertrayicon.cpp

   # base widget for activatables
   activatableitem.cpp

   # subclasses for Activatable subtypes
   interfaceconnectionitem.cpp
   unconfiguredinterfaceitem.cpp
   wirelessinterfaceconnectionitem.cpp
   vpninterfaceconnectionitem.cpp
   wirelessnetworkitem.cpp

   # helper class for wireless UI elements
   wirelessstatus.cpp

   # helper class
   tooltipbuilder.cpp

   # other networks dialog
   otherwirelessnetworkdialog.cpp

   # connection info
   connectioninfodialog.cpp
   connectioninfoipv4tab.cpp
   connectioninfowiredtab.cpp
   connectioninfowirelesstab.cpp

   # static global definitions of event string ids
   ../libs/service/events.cpp
   )

kde4_add_ui_files(monolithic_SRCS
   otherwirelessnetworkwidget.ui
   connectioninfowidget.ui
   )

kde4_add_executable(knetworkmanager ${monolithic_SRCS})


if (${KDE_VERSION} VERSION_LESS "4.3.67")
   set(KNOTIFICATIONITEM_LIB "knotificationitem-1")
endif (${KDE_VERSION} VERSION_LESS "4.3.67")

target_link_libraries(knetworkmanager
   ${KDE4_KDECORE_LIBS}
   ${KDE4_KDEUI_LIBS}
   ${KNOTIFICATIONITEM_LIB}
   ${QT_QTNETWORK_LIBRARY}
   knminternals
   knmservice
   knm_nm
   solid
   solidcontrol
   )

install(TARGETS knetworkmanager ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS knetworkmanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

if (INSTALL_KNM_AUTOSTART)
    install(FILES kde4-knetworkmanager-autostart.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
endif (INSTALL_KNM_AUTOSTART)

