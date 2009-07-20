add_subdirectory(icons)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/service
   ${CMAKE_CURRENT_SOURCE_DIR}/../backends/NetworkManager
   )


set(monolithic_SRCS
   # base client code
   monolithic.cpp

   # sort logic for activatables
   sortedactivatablelist.cpp

   # tray icon
   knetworkmanagertrayicon.cpp

   # base widget for activatables
   activatableitem.cpp

   # subclasses for Activatable subtypes
   interfaceconnectionitem.cpp
   unconfiguredinterfaceitem.cpp
   wirelessinterfaceconnectionitem.cpp
   vpninterfaceconnectionitem.cpp
   wirelessnetworkitemitem.cpp

   # helper class for wireless UI elements
   wirelessstatus.cpp

   # helper class
   tooltipbuilder.cpp
   )

kde4_add_executable(knetworkmanager ${monolithic_SRCS})

target_link_libraries(knetworkmanager
   ${KDE4_KDECORE_LIBS}
   ${KDE4_KDEUI_LIBS}
   knotificationitem-1
   knminternals
   knmservice
   knm_nm
   )

install(TARGETS knetworkmanager ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES knetworkmanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

install(FILES kde4-knetworkmanager-autostart.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
