set(networkmanager_SRCS
    events.cpp
    connectionitem.cpp
    connectioninspector.cpp
    networkmanager.cpp
    networkmanagerpopup.cpp
    interfacegroup.cpp
    interfaceitem.cpp
    nm-active-connectioninterface.cpp
    nm-settingsinterface.cpp
    nm-exported-connectioninterface.cpp
    remoteconnection.cpp
    networkmanagersettings.cpp
    wirelessinterfaceitem.cpp
    wirelessnetwork.cpp
    )

kde4_add_plugin(plasma_applet_networkmanager ${networkmanager_SRCS})
target_link_libraries(plasma_applet_networkmanager plasma ${KDE4_KIO_LIBS} solidcontrol)

install(TARGETS plasma_applet_networkmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES networkmanager.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/)

