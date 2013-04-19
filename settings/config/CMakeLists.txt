add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51013)

include_directories(../../libs)
include_directories(../../libs/internals)
include_directories(../../libs/service)
include_directories(../../libs/ui)
include_directories(../../backends/NetworkManager)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libs/internals)

## next target: connection editor KCM

set(kcm_networkmanagement_SRCS
   # UI logic
   addeditdeletebuttonset.cpp
   manageconnectionwidget.cpp
   mobileconnectionwizard.cpp
   mobileproviders.cpp
   treewidgetitem.cpp
)
#kde4_add_kcfg_files(kcm_knetworkmanager_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage/knmserviceprefs.kcfgc)

kde4_add_ui_files(kcm_networkmanagement_SRCS manageconnectionwidget.ui)
kde4_add_plugin(kcm_networkmanagement ${kcm_networkmanagement_SRCS})

target_link_libraries(kcm_networkmanagement knm_nm knmui knminternals knmservice solidcontrolnm09 ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTXML_LIBRARY} ${KDE4_KNOTIFYCONFIG_LIBRARY} ${KDE4_KIO_LIBS} ${KDE4_KCMUTILS_LIBS})

## next target: tray config KCM

set(kcm_networkmanagement_tray_SRCS
   addeditdeletebuttonset.cpp
   managetraywidget.cpp
   othersettingswidget.cpp
   )

kde4_add_ui_files(kcm_networkmanagement_tray_SRCS othersettings.ui othersettings.ui)
kde4_add_plugin(kcm_networkmanagement_tray ${kcm_networkmanagement_tray_SRCS})

target_link_libraries(kcm_networkmanagement_tray knminternals knmservice ${KDE4_KDEUI_LIBS} ${KDE4_KNOTIFYCONFIG_LIBRARY})

install(TARGETS kcm_networkmanagement kcm_networkmanagement_tray DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kcm_networkmanagement.desktop kcm_networkmanagement_tray.desktop DESTINATION ${SERVICES_INSTALL_DIR})

