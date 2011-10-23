include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals)

set(pptp_SRCS
   pptp.cpp
   pptpwidget.cpp
   pptpauth.cpp
   )

kde4_add_ui_files(pptp_SRCS pptpprop.ui pptpadvanced.ui pptpauth.ui)

kde4_add_plugin(networkmanagement_pptpui ${pptp_SRCS})

target_link_libraries(networkmanagement_pptpui ${KDE4_KIO_LIBS} knminternals knmui)

install(TARGETS networkmanagement_pptpui  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES networkmanagement_pptpui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

