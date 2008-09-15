include_directories(../../libs)
include_directories(../../libs/storage)
include_directories(../../libs/ui)
## next target: wired KCM

# just a proof of concept, disabled now because the knmui library needs cleaning up into widgets
set(kcm_knetworkmanager_wired_SRCS
   # UI logic
   wiredpreferences.cpp
   )
kde4_add_plugin(kcm_knetworkmanager_wired ${kcm_knetworkmanager_wired_SRCS})

target_link_libraries(kcm_knetworkmanager_wired knmstorage knmui ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY})

install(TARGETS kcm_knetworkmanager_wired DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_knetworkmanager_wired.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)

## next target: connection editor KCM

set(kcm_knetworkmanager_SRCS
   # UI logic
   connectioneditor.cpp)

kde4_add_ui_files(kcm_knetworkmanager_SRCS connectioneditor.ui)
kde4_add_plugin(kcm_knetworkmanager ${kcm_knetworkmanager_SRCS})

target_link_libraries(kcm_knetworkmanager ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_knetworkmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_knetworkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)



