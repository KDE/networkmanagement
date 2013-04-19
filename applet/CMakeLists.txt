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
    connectionslistmodel.cpp
    connectionitem.cpp
    declarativenmpopup.cpp
    declarativeinterfaceitem.cpp
    interfaceslistmodel.cpp
    interfacedetailswidget.cpp
    ../libs/service/events.cpp
    )

kde4_add_plugin(plasma_applet_networkmanagement ${networkmanager_SRCS})

target_link_libraries(plasma_applet_networkmanagement ${KDE4_PLASMA_LIBS}
        knmclient knminternals knm_nm ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS}
        ${QT_QTDECLARATIVE_LIBRARY} ModemManagerQt NetworkManagerQt)

install(TARGETS plasma_applet_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(DIRECTORY qml/ DESTINATION ${DATA_INSTALL_DIR}/networkmanagement/qml)

add_subdirectory(icons)

install(FILES
        network2.svgz
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/icons/)

target_link_libraries(plasma_applet_networkmanagement ${KDE4_KCMUTILS_LIBS})
