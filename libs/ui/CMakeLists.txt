include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_SOURCE_DIR}/../internals
   ${CMAKE_CURRENT_BINARY_DIR}/security
   ${CMAKE_CURRENT_BINARY_DIR}/../internals
   )
## next target: ui library

set(knmui_SRCS
   cdmawidget.cpp
   gsmwidget.cpp
   pppwidget.cpp
   pppoewidget.cpp
   security/802_1x_security_widget.cpp
   security/securitywidget.cpp
   settinginterface.cpp
   settingwidget.cpp
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

   # top level wireless security widget
   security/wirelesssecuritysettingwidget.cpp
   security/nullsecuritywidget.cpp
   security/wepwidget.cpp
   security/wpapskwidget.cpp
   security/wpaeapwidget.cpp
   security/eapmethod.cpp
   security/eapmethodstack.cpp
   security/eapmethodsimple.cpp
   # intermediate subclass for eap methods that use inner auth
   security/eapmethodinnerauth.cpp

   security/eapmethodleap.cpp
   security/ttlswidget.cpp
   security/peapwidget.cpp
   security/tlswidget.cpp

   vpnuiplugin.cpp
   connectionprefs.cpp
   gsmconnectioneditor.cpp
   cdmaconnectioneditor.cpp
   pppoepreferences.cpp
   vpnpreferences.cpp
   wiredpreferences.cpp
   wirelesspreferences.cpp
   connectioneditor.cpp
   connectionsecretsjob.cpp
   )

kde4_add_ui_files(knmui_SRCS
   #802_1x.ui
   #security.ui

   # Settings
   cdma.ui
   gsm.ui
   ppp.ui
   pppoe.ui
   connection.ui
   wired.ui
   ipv4.ui
   802-11-wireless.ui

   # Scan widget for wireless
   scanwidget.ui

   # base wireless security type selector
   security/wirelesssecurity.ui
   # wireless security type specific UIs
   security/wep.ui
   security/wpapsk.ui
   security/802_1x.ui
   #   WPA-EAP - a stack of eap methods UI
   #   Used for both inner and outer auth methods
   security/eapmethodstackbase.ui

   # simple eap methods eg MD5 and *AP UI
   security/eapmethodsimplebase.ui
   # LEAP
   security/eapmethodleapbase.ui
   # PEAP
   security/eapmethodpeapbase.ui
   # TLS
   security/eapmethodtlsbase.ui
   # TTLS
   security/eapmethodttlsbase.ui
   )

kde4_add_library(knmui SHARED ${knmui_SRCS})
target_link_libraries(knmui knminternals ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} solidcontrol)

set_target_properties(knmui PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )
install(TARGETS knmui ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES networkmanagement_vpnuiplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

