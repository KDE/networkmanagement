include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libs/client
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
   )

add_subdirectory(pics)

set(networkmanager_SRCS
    activatableitem.cpp

    interfaceconnectionitem.cpp

    networkmanager.cpp

    interfaceitem.cpp
    #serialinterfaceitem.cpp
    events.cpp
    #vpnconnectiongroup.cpp

    wiredinterfaceitem.cpp

    wirelessinterfaceitem.cpp
    wirelessnetworkitem.cpp

    nmextenderitem.cpp
    activatablelistwidget.cpp
    )

kde4_add_ui_files(networkmanager_SRCS nmConfig.ui )
if(NOT DEFINED ${I_KNOW_WHAT_IM_DOING})
   set(networkmanager_SRCS applet_wip.cpp)
   message(STATUS ${networkmanager_SRCS})
endif(NOT DEFINED ${I_KNOW_WHAT_IM_DOING})

kde4_add_plugin(plasma_applet_networkmanagement ${networkmanager_SRCS})
target_link_libraries(plasma_applet_networkmanagement ${KDE4_PLASMA_LIBS} knmclient knminternals ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)

install(TARGETS plasma_applet_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES networkmanagement.svg networkmanagement-wireless.svgz DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanagement/)

