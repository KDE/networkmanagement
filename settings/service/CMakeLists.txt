include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/dbus
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui
   )
   ## next target: test service

set(kded_knetworkmanager_SRCS
   datamappings.cpp
   service.cpp
   busconnection.cpp
   connectionsecretsjob.cpp
   exportedconnection.cpp
   kconfigtoservice.cpp
   networksettings.cpp
   )

kde4_add_kcfg_files(kded_knetworkmanager_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage/knmserviceprefs.kcfgc)

qt4_add_dbus_adaptor(kded_knetworkmanager_SRCS org.kde.knetworkmanagerd.xml kconfigtoservice.h KConfigToService knetworkmanagerserviceadaptor KNetworkManagerServiceAdaptor )

kde4_add_plugin(kded_knetworkmanager ${kded_knetworkmanager_SRCS})
target_link_libraries(kded_knetworkmanager knmdbus knmstorage knmui ${KDE4_KUTILS_LIBS} solidcontrol)

install(TARGETS kded_knetworkmanager DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES knetworkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)

