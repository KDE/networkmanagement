project(networkconnections)

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.networkconnections PATTERN .svn EXCLUDE)

install(FILES plasma-applet-networkconnections.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
