include_directories(../../libs)
include_directories(../../libs/internals)
include_directories(../../libs/ui)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libs/internals)

## next target: connection editor KCM

set(kcm_networkmanagement_SRCS
   # UI logic
   manageconnectionwidget.cpp
)
#kde4_add_kcfg_files(kcm_knetworkmanager_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage/knmserviceprefs.kcfgc)

kde4_add_ui_files(kcm_networkmanagement_SRCS manageconnectionwidget.ui)
kde4_add_plugin(kcm_networkmanagement ${kcm_networkmanagement_SRCS})

target_link_libraries(kcm_networkmanagement knmdbus knmui knmstorage solidcontrol ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})

install(TARGETS kcm_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcm_networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR})

