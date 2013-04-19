
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libs/client
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/ui
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/../libs/internals
   )

#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

set(networkmanagement_engine_srcs
    ../applet/wirelessstatus.cpp
    networkmanagementengine.cpp
)

kde4_add_plugin(plasma_engine_networkmanagement ${networkmanagement_engine_srcs})

target_link_libraries(plasma_engine_networkmanagement
    ${KDE4_KDECORE_LIBS}
    ${KDE4_PLASMA_LIBS}
    knmclient knminternals
    ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS}
    solidcontrolnm09
    ${KDE4_KCMUTILS_LIBS}
)

install(TARGETS plasma_engine_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR})

