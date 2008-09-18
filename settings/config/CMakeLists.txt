include_directories(../../libs)
include_directories(../../libs/storage)
include_directories(../../libs/ui)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## next target: connection editor KCM

set(kcm_knetworkmanager_SRCS
   # UI logic
   connectioneditor.cpp
   connectionprefs.cpp
   cellularpreferences.cpp
   pppoepreferences.cpp
   wiredpreferences.cpp
   wirelesspreferences.cpp
)
kde4_add_kcfg_files(kcm_knetworkmanager_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage/knmserviceprefs.kcfgc)

kde4_add_ui_files(kcm_knetworkmanager_SRCS connectioneditor.ui)
kde4_add_plugin(kcm_knetworkmanager ${kcm_knetworkmanager_SRCS})

target_link_libraries(kcm_knetworkmanager knmui knmstorage ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})

install(TARGETS kcm_knetworkmanager DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcm_knetworkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR})

