set(networkmanager_SRCS
    networkmanager.cpp
    networkmanagerpopup.cpp)

kde4_add_plugin(plasma_applet_networkmanager_alpha ${networkmanager_SRCS})
target_link_libraries(plasma_applet_networkmanager_alpha plasma ${KDE4_KIO_LIBS} solidcontrol)

install(TARGETS plasma_applet_networkmanager_alpha DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanager_alpha.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES networkmanager_alpha.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/)

