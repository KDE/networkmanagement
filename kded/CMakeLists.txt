include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/
   ${CMAKE_CURRENT_SOURCE_DIR}/../solidcontrolfuture
   )
   ## next target: test service

set(kded_networkmanagement_SRCS
   service.cpp
   )

kde4_add_kcfg_files(kded_networkmanagement_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals/knmserviceprefs.kcfgc)

qt4_add_dbus_adaptor(kded_networkmanagement_SRCS org.kde.networkmanagement.xml service.h NetworkManagementService )

kde4_add_plugin(kded_networkmanagement ${kded_networkmanagement_SRCS})
target_link_libraries(kded_networkmanagement knminternals knmexternals ${KDE4_KUTILS_LIBS} solidcontrol solidcontrolfuture)

install(TARGETS kded_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES knetworkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)

