include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/dbus
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../../backends/NetworkManager
   )
   ## next target: test service

set(kded_networkmanagement_SRCS
   datamappings.cpp
   service.cpp
   busconnection.cpp
   connectionsecretsjob.cpp
   exportedconnection.cpp
   kconfigtoservice.cpp
   networksettings.cpp
   )

kde4_add_kcfg_files(kded_networkmanagement_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals/knmserviceprefs.kcfgc)

qt4_add_dbus_adaptor(kded_networkmanagement_SRCS org.kde.networkmanagement.xml kconfigtoservice.h KConfigToService knetworkmanagerserviceadaptor KNetworkManagerServiceAdaptor )

kde4_add_plugin(kded_networkmanagement ${kded_networkmanagement_SRCS})
target_link_libraries(kded_networkmanagement knmdbus knmstorage knmui ${KDE4_KUTILS_LIBS} solidcontrol knm_nm)

install(TARGETS kded_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES knetworkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)

