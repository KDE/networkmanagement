include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals)

set(vpnc_SRCS
   vpnc.cpp
   vpncauth.cpp
   vpncwidget.cpp
   )

kde4_add_ui_files(vpnc_SRCS vpncprop.ui vpncauth.ui)

kde4_add_plugin(networkmanagement_vpncui ${vpnc_SRCS})

target_link_libraries(networkmanagement_vpncui solidcontrolnm09 ${KDE4_KIO_LIBS} knminternals knmui)

install(TARGETS networkmanagement_vpncui  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES networkmanagement_vpncui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

