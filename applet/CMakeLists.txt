include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dbus
   )

add_subdirectory(pics)

set(networkmanager_SRCS
    abstractconnectableitem.cpp
    generalextender.cpp
    connectionitem.cpp
    connectioninspector.cpp
    connectionlist.cpp
    hiddenwirelessnetworkitem.cpp
    mergedwireless.cpp
    networkmanager.cpp
    interfacegroup.cpp
    interfaceitem.cpp
    serialinterfaceitem.cpp
    events.cpp
    vpnconnectiongroup.cpp
    wiredinterfaceitem.cpp
    wirelessconnectionitem.cpp
    wirelessenvironment.cpp
    wirelessinterfaceitem.cpp
    wirelessnetwork.cpp
    wirelessnetworkitem.cpp
    )

kde4_add_ui_files(networkmanager_SRCS nmConfig.ui )
kde4_add_plugin(plasma_applet_networkmanagement ${networkmanager_SRCS})
target_link_libraries(plasma_applet_networkmanagement ${KDE4_PLASMA_LIBS} knmdbus ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)

install(TARGETS plasma_applet_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES networkmanagement.svg networkmanagement-wireless.svgz DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanagement/)

