add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51010)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../../solidcontrolfuture
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/client
   ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/NetworkManager
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs
   )

## next target: internals layer library

set(knminternals_SRCS
   connection.cpp

   setting.cpp

   #settings/pbkdf2.cpp
   #settings/wephash.cpp

   settings/802-11-wireless-security.cpp

   settings/802-11-wireless.cpp

   settings/802-3-ethernet.cpp

   settings/802-1x.cpp

   settings/gsm.cpp

   settings/cdma.cpp

   settings/ipv4.cpp

   settings/ipv6.cpp

   settings/ppp.cpp

   settings/pppoe.cpp

   settings/serial.cpp

   settings/vpn.cpp

   settings/bluetooth.cpp

   activatable.cpp
   interfaceconnection.cpp
   interfaceconnectionhelpers.cpp
   hiddenwirelessinterfaceconnection.cpp
   wirelessinterfaceconnection.cpp
   wirelessinterfaceconnectionhelpers.cpp
   wirelessobject.cpp
   wirelessnetwork.cpp
   unconfiguredinterface.cpp
   vpninterfaceconnection.cpp
   vpninterfaceconnectionhelpers.cpp
   gsminterfaceconnection.cpp
   gsminterfaceconnectionhelpers.cpp

   # helper class for recognising the type of WPA secrets
   wpasecretidentifier.cpp

   # helper class for identifying wireless security
   wirelesssecurityidentifier.cpp

   # helper functions for storing tooltip keys and UI strings
   tooltips.cpp
   # helper functions to obtain UI strings for various enums
   uiutils.cpp
   )

kde4_add_kcfg_files(knminternals_SRCS knmserviceprefs.kcfgc)

kde4_add_library(knminternals SHARED ${knminternals_SRCS})

target_link_libraries(knminternals knmclient solidcontrolfuture ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${KDE4_SOLID_LIBS} solidcontrolnm09 ${QT_QTNETWORK_LIBRARY})

set_target_properties(knminternals PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )
install(TARGETS knminternals ${INSTALL_TARGETS_DEFAULT_ARGS})

set(kconfig_compiler_SRCS kconfig_compiler.cpp)

kde4_add_executable(knm_compiler NOGUI RUN_UNINSTALLED ${kconfig_compiler_SRCS})

target_link_libraries(knm_compiler  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )

