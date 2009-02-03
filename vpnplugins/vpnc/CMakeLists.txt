include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage)

set(vpnc_SRCS
   vpnc.cpp
   vpncwidget.cpp
   )

kde4_add_ui_files(vpnc_SRCS vpncprop.ui)

kde4_add_plugin(networkmanagement_vpncui ${vpnc_SRCS})

target_link_libraries(networkmanagement_vpncui ${KDE4_KIO_LIBS} knmui)

install(TARGETS networkmanagement_vpncui  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES networkmanagement_vpncui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

