include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)

set(vpnc_SRCS
   vpnc.cpp
   vpncwidget.cpp
   )

kde4_add_ui_files(vpnc_SRCS vpncprop.ui)

kde4_add_plugin(knetworkmanager4_vpncui ${vpnc_SRCS})

target_link_libraries(knetworkmanager4_vpncui ${KDE4_KIO_LIBS} knmui)

install(TARGETS knetworkmanager4_vpncui  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES knetworkmanager4_vpncui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

