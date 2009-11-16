include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/client
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/ui
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/../libs/internals
   )

set(networkmanager_SRCS
    networkmanager.cpp

    nmextenderitem.cpp
    activatablelistwidget.cpp

    interfaceitem.cpp
    wiredinterfaceitem.cpp
    wirelessinterfaceitem.cpp

    activatableitem.cpp
    interfaceconnectionitem.cpp
    wirelessnetworkitem.cpp
    )

kde4_add_plugin(plasma_applet_networkmanagement ${networkmanager_SRCS})
target_link_libraries(plasma_applet_networkmanagement ${KDE4_PLASMA_LIBS} knmclient knminternals ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)

install(TARGETS plasma_applet_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR})

