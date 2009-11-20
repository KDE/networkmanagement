include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals)

set(novellvpn_SRCS
   novellvpn.cpp
   novellvpnwidget.cpp
   )

kde4_add_ui_files(novellvpn_SRCS novellvpnprop.ui novellvpnadvancedprop.ui)

kde4_add_plugin(networkmanagement_novellvpnui ${novellvpn_SRCS})

target_link_libraries(networkmanagement_novellvpnui ${KDE4_KIO_LIBS} knmui knminternals )

install(TARGETS networkmanagement_novellvpnui  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES networkmanagement_novellvpnui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

