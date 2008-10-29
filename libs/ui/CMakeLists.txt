include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_SOURCE_DIR}/../storage
   )
## next target: ui library

add_subdirectory(vpnplugins)

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
   accesspoint.cpp
   apitemdelegate.cpp
   apitemmodel.cpp
   apitemview.cpp
   ifaceitemmodel.cpp
   scanwidget.cpp
   security/802_11_wireless_security_widget.cpp
   security/wepwidget.cpp
   security/wpapskwidget.cpp
   vpnuiplugin.cpp
   )

kde4_add_ui_files(knmui_SRCS cdma.ui gsm.ui ppp.ui pppoe.ui 
   #802_1x.ui #security.ui 
   connection.ui wired.ui ipv4.ui 802-11-wireless.ui 
   scanwidget.ui
   security/802_11_wireless_security.ui
   security/wep.ui
   security/wpapsk.ui
   security/802_1x.ui
   )

kde4_add_library(knmui SHARED ${knmui_SRCS})
target_link_libraries(knmui knmstorage ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} solidcontrol)

set_target_properties(knmui PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )
install(TARGETS knmui ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES knetworkmanager4_vpnuiplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

