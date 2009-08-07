include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libs/client
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
   )

add_subdirectory(pics)

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

kde4_add_ui_files(networkmanager_SRCS nmConfig.ui )

if(NOT I_KNOW_WHAT_IM_DOING)
   set(networkmanager_SRCS applet_wip.cpp)
   message(STATUS "Compiling dummy plasmoid because the real one is still work in progress...")
endif(NOT I_KNOW_WHAT_IM_DOING)

kde4_add_plugin(plasma_applet_networkmanagement ${networkmanager_SRCS})
target_link_libraries(plasma_applet_networkmanagement ${KDE4_PLASMA_LIBS} knmclient knminternals ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)

install(TARGETS plasma_applet_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES networkmanagement.svg networkmanagement-wireless.svgz DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanagement/)

