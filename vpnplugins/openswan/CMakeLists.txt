include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui/security)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libs/internals)

set(openswan_SRCS
   openswan.cpp
   openswanwidget.cpp
   openswanauth.cpp
   )

kde4_add_ui_files(openswan_SRCS openswan.ui openswanauth.ui)

kde4_add_plugin(networkmanagement_openswanui ${openswan_SRCS})

target_link_libraries(networkmanagement_openswanui ${KDE4_KIO_LIBS} knminternals knmui)

install(TARGETS networkmanagement_openswanui  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES networkmanagement_openswanui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
