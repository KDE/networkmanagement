include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/client
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/ui
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs
                    ${CMAKE_CURRENT_SOURCE_DIR}/../backends/NetworkManager
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/../libs/internals
   )

set(networkmanager_SRCS
    networkmanager.cpp

    nmpopup.cpp
    activatablelistwidget.cpp

    interfaceitem.cpp
    wirelessinterfaceitem.cpp
    vpninterfaceitem.cpp
    interfacedetailswidget.cpp

    activatableitem.cpp
    interfaceconnectionitem.cpp
    wirelessnetworkitem.cpp
    hiddenwirelessnetworkitem.cpp
    wirelessstatus.cpp
    gsminterfaceconnectionitem.cpp
    )

kde4_add_plugin(plasma_applet_networkmanagement ${networkmanager_SRCS})

target_link_libraries(plasma_applet_networkmanagement ${KDE4_PLASMA_LIBS} knmclient knminternals ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)

install(TARGETS plasma_applet_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)

# We only install the theme for the version where it isn't in SVN
macro_ensure_version("4.4.76" ${KDE_VERSION} IS45)
if (NOT IS45)
    install(FILES
        network.svgz
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/icons/)
    target_link_libraries(plasma_applet_networkmanagement ${KDE4_KUTILS_LIBS})
else (NOT IS45)
    target_link_libraries(plasma_applet_networkmanagement ${KDE4_KCMUTILS_LIBS})
endif (NOT IS45)
