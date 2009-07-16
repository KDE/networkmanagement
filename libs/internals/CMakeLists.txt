include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   )

## next target: internals layer library

set(knminternals_SRCS
   connection.cpp
   #connectiondbus.cpp
   connectionpersistence.cpp

   setting.cpp
   #settingdbus.cpp
   settingpersistence.cpp

   #settings/pbkdf2.cpp
   #settings/wephash.cpp

   settings/802-11-wireless-security.cpp
   #settings/802-11-wireless-securitydbus.cpp
   settings/802-11-wireless-securitypersistence.cpp

   settings/802-11-wireless.cpp
   #settings/802-11-wirelessdbus.cpp
   settings/802-11-wirelesspersistence.cpp

   settings/802-3-ethernet.cpp
   #settings/802-3-ethernetdbus.cpp
   settings/802-3-ethernetpersistence.cpp

   settings/802-1x.cpp
   #settings/802-1xdbus.cpp
   settings/802-1xpersistence.cpp

   settings/gsm.cpp
   #settings/gsmdbus.cpp
   settings/gsmpersistence.cpp

   settings/cdma.cpp
   #settings/cdmadbus.cpp
   settings/cdmapersistence.cpp

   settings/ipv4.cpp
   #settings/ipv4dbus.cpp
   settings/ipv4persistence.cpp

   settings/ppp.cpp
   #settings/pppdbus.cpp
   settings/ppppersistence.cpp

   settings/pppoe.cpp
   #settings/pppoedbus.cpp
   settings/pppoepersistence.cpp

   settings/serial.cpp
   #settings/serialdbus.cpp
   settings/serialpersistence.cpp

   settings/vpn.cpp
   #settings/vpndbus.cpp
   settings/vpnpersistence.cpp

   activatable.cpp
   interfaceconnection.cpp
   wirelessinterfaceconnection.cpp
   wirelessitem.cpp
   wirelessnetworkitem.cpp
   unconfiguredinterface.cpp
   vpninterfaceconnection.cpp
   )

kde4_add_kcfg_files(knminternals_SRCS knmserviceprefs.kcfgc)

kde4_add_library(knminternals SHARED ${knminternals_SRCS})

target_link_libraries(knminternals ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} solidcontrol ${QT_QTNETWORK_LIBRARY})

set_target_properties(knminternals PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )
install(TARGETS knminternals ${INSTALL_TARGETS_DEFAULT_ARGS})

set(kconfig_compiler_SRCS kconfig_compiler.cpp)

kde4_add_executable(knm_compiler NOGUI RUN_UNINSTALLED ${kconfig_compiler_SRCS})

target_link_libraries(knm_compiler  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )

