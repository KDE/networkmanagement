include_directories(
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs
   ${CMAKE_CURRENT_SOURCE_DIR}/dbus
   ${CMAKE_CURRENT_BUILD_DIR}
)

#add_subdirectory(dbus)

set(solid_modemmanager_PART_SRCS
   manager.cpp
   modeminterface.cpp
   modemlocationinterface.cpp
   modemcdmainterface.cpp
   modemgsmcardinterface.cpp
   modemgsmcontactsinterface.cpp
   modemgsmnetworkinterface.cpp
   modemgsmsmsinterface.cpp
   modemgsmhsointerface.cpp
   modemgsmussdinterface.cpp
)

set( DBUS_INTERFACES_FILES
   dbus/generic-types.cpp
   dbus/mm-manager-clientinterface.cpp
   dbus/mm-modeminterface.cpp
   dbus/mm-modem-simpleinterface.cpp
   dbus/mm-modem-locationinterface.cpp
   dbus/mm-modem-cdmainterface.cpp
   dbus/mm-modem-gsm-cardinterface.cpp
   dbus/mm-modem-gsm-contactsinterface.cpp
   dbus/mm-modem-gsm-networkinterface.cpp
   dbus/mm-modem-gsm-smsinterface.cpp
   dbus/mm-modem-gsm-hsointerface.cpp
   dbus/mm-modem-gsm-ussdinterface.cpp
)


kde4_add_library(solid_modemmanager05_static STATIC ${solid_modemmanager_PART_SRCS} ${DBUS_INTERFACES_FILES})
target_link_libraries(solid_modemmanager05_static ${KDE4_KDECORE_LIBS} solidcontrolnm09ifaces)

kde4_add_plugin(solid_modemmanager05 module.cpp ${solid_modemmanager_PART_SRCS} ${DBUS_INTERFACES_FILES})

target_link_libraries(solid_modemmanager05 ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrolnm09 solidcontrolnm09ifaces)
install(TARGETS solid_modemmanager05 DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_modemmanager05.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )

kde4_install_icons( ${ICON_INSTALL_DIR} )
