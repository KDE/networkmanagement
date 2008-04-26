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
