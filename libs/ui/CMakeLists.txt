add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51012)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_SOURCE_DIR}/../../solidcontrolfuture
   ${CMAKE_CURRENT_SOURCE_DIR}/../internals
   ${CMAKE_CURRENT_BINARY_DIR}/security
   ${CMAKE_CURRENT_BINARY_DIR}/../internals
   )
## next target: ui library

set(knmui_SRCS
   cdmawidget.cpp
   gsmwidget.cpp
   bluetoothwidget.cpp
   pppwidget.cpp
   pppoewidget.cpp
   security/securitywidget.cpp
   settingwidget.cpp
   connectionwidget.cpp
   ipv4widget.cpp
   ipv4advancedwidget.cpp
   ipv4routeswidget.cpp
   ipv4delegate.cpp
   ipv6widget.cpp
   ipv6advancedwidget.cpp
   ipv6routeswidget.cpp
   ipv6delegate.cpp
   wiredwidget.cpp
   802_11_wirelesswidget.cpp
   accesspoint.cpp
   apitemdelegate.cpp
   apitemmodel.cpp
   networkitemmodel.cpp
   apitemview.cpp
   scanwidget.cpp
   advancedpermissionswidget.cpp

   # top level wireless security widget
   security/wirelesssecuritysettingwidget.cpp
   security/nullsecuritywidget.cpp
   security/wepwidget.cpp
   security/securityleap.cpp
   security/wpapskwidget.cpp
   security/securitydynamicwep.cpp
   security/securitywired8021x.cpp
   # base UI for 802.1x
   security/securityeap.cpp
   # WPA-EAP
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

   # auth widgets for interactive password retrieval
   security/security8021xauth.cpp
   security/wirelesssecurityauth.cpp
   security/wepauthwidget.cpp
   security/wpaauthwidget.cpp
   security/leapauthwidget.cpp

   vpnuiplugin.cpp
   connectionprefs.cpp
   gsmconnectioneditor.cpp
   bluetoothconnectioneditor.cpp
   cdmaconnectioneditor.cpp
   pppoepreferences.cpp
   vpnpreferences.cpp
   wiredpreferences.cpp
   wirelesspreferences.cpp
   connectioneditor.cpp
   connectionsecretsjob.cpp

   simpleipv4addressvalidator.cpp
   simpleipv6addressvalidator.cpp
   listvalidator.cpp
   intvalidator.cpp
   editlistdialog.cpp
   delegate.cpp
   intdelegate.cpp
   )

kde4_add_ui_files(knmui_SRCS
   # Settings
   cdma.ui
   gsm.ui
   bluetooth.ui
   ppp.ui
   pppoe.ui
   connection.ui
   wired.ui
   ipv4.ui
   ipv4advanced.ui
   ipv4routes.ui
   ipv6.ui
   ipv6advanced.ui
   ipv6routes.ui
   802-11-wireless.ui

   # Scan widget for wireless
   scanwidget.ui

   # Advanced permissions editor
   advancedpermissionswidget.ui

   # base wireless security type selector
   security/wirelesssecurity.ui
   # wireless security type specific UIs
   security/wep.ui
   security/leap.ui
   security/wpapsk.ui
   security/wirelesssecurityauth.ui
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
target_link_libraries(knmui knminternals ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} solidcontrolnm09 solidcontrolfuture)

set_target_properties(knmui PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )
install(TARGETS knmui ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES networkmanagement_vpnuiplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

