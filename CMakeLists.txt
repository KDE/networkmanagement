project(plasma-networkmanager)

set(networkmanager_SRCS
    networkmanager.cpp
    nmmenu.cpp)

kde4_add_ui_files(networkmanager_SRCS )
kde4_add_plugin(plasma_applet_networkmanager ${networkmanager_SRCS})
target_link_libraries(plasma_applet_networkmanager plasma ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_networkmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES networkmanager.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/)
install(FILES icons/cr22-action-modem.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr22-action-nm_adhoc.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr22-action-nm_device_wired.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr22-action-nm_no_connection.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr22-action-nm_signal_00.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr22-action-nm_signal_25.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr22-action-nm_signal_50.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr22-action-nm_signal_75.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr22-action-nm_signal_100.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr22-action-wired.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)
install(FILES icons/cr32-app-knetworkmanager.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/icons)

