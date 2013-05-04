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
    ../solidcontrolfuture/solid/networkmanager-0.9/dbus/nm-dhcp4-configinterface.cpp

    ../libs/service/events.cpp
    )

kde4_add_plugin(plasma_applet_networkmanagement ${networkmanager_SRCS})

target_link_libraries(plasma_applet_networkmanagement ${KDE4_PLASMA_LIBS} knmclient knminternals knm_nm ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} solidcontrolnm09 solidcontrolfuture)

install(TARGETS plasma_applet_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)

install(FILES
        network2.svgz
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/icons/)

target_link_libraries(plasma_applet_networkmanagement ${KDE4_KCMUTILS_LIBS})
