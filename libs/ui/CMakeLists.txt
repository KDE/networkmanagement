include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_SOURCE_DIR}/../storage
   )
## next target: ui library

set(knmui_SRCS
   cdmawidget.cpp
   gsmwidget.cpp
   pppwidget.cpp
   pppoewidget.cpp
   security/802_1x_security_widget.cpp
   security/securitywidget.cpp
   settingwidget.cpp
   connectionwidgetfactory.cpp
   connectionwidget.cpp
   ipv4widget.cpp
   wiredwidget.cpp
   802_11_wirelesswidget.cpp
   security/802_11_wireless_security_widget.cpp
   security/wepwidget.cpp
   security/wpapskwidget.cpp
   )

kde4_add_ui_files(knmui_SRCS cdma.ui gsm.ui ppp.ui pppoe.ui 
   #802_1x.ui #security.ui 
   connection.ui wired.ui ipv4.ui 802-11-wireless.ui 
   security/802_11_wireless_security.ui
   security/wep.ui
   security/wpapsk.ui
   )

kde4_add_library(knmui SHARED ${knmui_SRCS})
target_link_libraries(knmui knmstorage ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

set_target_properties(knmui PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )
install(TARGETS knmui ${INSTALL_TARGETS_DEFAULT_ARGS})


