include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)

set(openvpn_SRCS
   openvpn.cpp
   openvpnwidget.cpp
   )

kde4_add_ui_files(openvpn_SRCS openvpnprop.ui)

kde4_add_plugin(knetworkmanager4_openvpnui ${openvpn_SRCS})

target_link_libraries(knetworkmanager4_openvpnui ${KDE4_KIO_LIBS} knmui knmstorage )

install(TARGETS knetworkmanager4_openvpnui  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES knetworkmanager4_openvpnui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

