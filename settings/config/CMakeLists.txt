include_directories(../../libs)
include_directories(../../libs/storage)
include_directories(../../libs/ui)

## next target: connection editor KCM

set(kcm_knetworkmanager_SRCS
   # UI logic
   connectioneditor.cpp)

kde4_add_ui_files(kcm_knetworkmanager_SRCS connectioneditor.ui)
kde4_add_plugin(kcm_knetworkmanager ${kcm_knetworkmanager_SRCS})

target_link_libraries(kcm_knetworkmanager ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_knetworkmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_knetworkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)


## next target: wired KCM

set(kcm_knetworkmanager_wired_SRCS
   wiredpreferences.cpp
   )
kde4_add_plugin(kcm_knetworkmanager_wired ${kcm_knetworkmanager_wired_SRCS})

target_link_libraries(kcm_knetworkmanager_wired knmui ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_knetworkmanager_wired DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_knetworkmanager_wired.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)

## next target: wireless KCM

set(kcm_knetworkmanager_wireless_SRCS
   wirelesspreferences.cpp
   )
kde4_add_plugin(kcm_knetworkmanager_wireless ${kcm_knetworkmanager_wireless_SRCS})

target_link_libraries(kcm_knetworkmanager_wireless knmui ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_knetworkmanager_wireless DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_knetworkmanager_wireless.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)

## next target: DSL KCM

set(kcm_knetworkmanager_pppoe_SRCS
   pppoepreferences.cpp
   )
kde4_add_plugin(kcm_knetworkmanager_pppoe ${kcm_knetworkmanager_pppoe_SRCS})

target_link_libraries(kcm_knetworkmanager_pppoe knmui ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_knetworkmanager_pppoe DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_knetworkmanager_pppoe.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)

## next target: Cellular KCM

set(kcm_knetworkmanager_cellular_SRCS
   cellularpreferences.cpp
   )
kde4_add_plugin(kcm_knetworkmanager_cellular ${kcm_knetworkmanager_cellular_SRCS})

target_link_libraries(kcm_knetworkmanager_cellular knmui ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_knetworkmanager_cellular DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_knetworkmanager_cellular.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)


