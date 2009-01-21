install(FILES 802-11-wireless.kcfg
   802-3-ethernet.kcfg
   gsm.kcfg
   ppp.kcfg
   vpn.kcfg
   802-11-wireless-security.kcfg
   cdma.kcfg
   ipv4.kcfg
   pppoe.kcfg
   802-1x.kcfg
   connection.kcfg
   serial.kcfg
   DESTINATION ${DATA_INSTALL_DIR}/knetworkmanager/schemas/)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage
   )
set(foo_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage/settings/802-11-wirelesspersistence.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage/settings/802-11-wirelessdbus.cpp
)
kde4_add_kcfg_files(foo_SRCS 802-11-wireless.kcfg)
kde4_add_library(foo ${foo_SRCS})

message(STATUS ${foo_SRCS})
