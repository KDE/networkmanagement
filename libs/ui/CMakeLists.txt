include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   )
## next target: ui library

set(knmui_SRCS
   settingwidget.cpp
   ipv4widget.cpp
   wiredwidget.cpp
   802_11_wirelesswidget.cpp
   802_11_wireless_security_widget.cpp
   )
kde4_add_ui_files(knmui_SRCS connection.ui wired.ui ipv4.ui 802-11-wireless.ui 802-11-wireless-security.ui)
kde4_add_library(knmui SHARED ${knmui_SRCS})
target_link_libraries(knmui ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

set_target_properties(knmui PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )
install(TARGETS knmui ${INSTALL_TARGETS_DEFAULT_ARGS})


