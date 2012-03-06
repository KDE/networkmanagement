include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals)

set(openvpn_SRCS
   openvpn.cpp
   openvpnwidget.cpp
   openvpnauth.cpp
   )

kde4_add_ui_files(openvpn_SRCS openvpnprop.ui)

kde4_add_plugin(networkmanagement_openvpnui ${openvpn_SRCS})

target_link_libraries(networkmanagement_openvpnui solidcontrolnm09 ${KDE4_KIO_LIBS} knmui knminternals )

install(TARGETS networkmanagement_openvpnui  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES networkmanagement_openvpnui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

