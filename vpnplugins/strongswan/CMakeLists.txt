include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui/security)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libs/internals)

set(strongswan_SRCS
   strongswan.cpp
   strongswanwidget.cpp
   strongswanauth.cpp
   )




kde4_add_ui_files(strongswan_SRCS strongswanprop.ui strongswanauth.ui)

kde4_add_plugin(networkmanagement_strongswanui ${strongswan_SRCS})

target_link_libraries(networkmanagement_strongswanui ${KDE4_KIO_LIBS} knminternals knmui)

install(TARGETS networkmanagement_strongswanui  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES networkmanagement_strongswanui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

