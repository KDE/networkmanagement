include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dbus
   )

set(networkmanager_SRCS
    abstractconnectableitem.cpp
    connectionitem.cpp
    connectioninspector.cpp
    connectionlist.cpp
    mergedwireless.cpp
    networkmanager.cpp
    networkmanagerpopup.cpp
    interfacegroup.cpp
    interfaceitem.cpp
    serialinterfaceitem.cpp
    events.cpp
    vpnconnectiongroup.cpp
    wirelessconnectionitem.cpp
    wirelessenvironment.cpp
    wirelessinterfaceitem.cpp
    wirelessnetwork.cpp
    wirelessnetworkitem.cpp
    )

kde4_add_plugin(plasma_applet_networkmanager ${networkmanager_SRCS})
target_link_libraries(plasma_applet_networkmanager plasma knmdbus ${KDE4_KIO_LIBS} solidcontrol)

install(TARGETS plasma_applet_networkmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES networkmanager.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/)

