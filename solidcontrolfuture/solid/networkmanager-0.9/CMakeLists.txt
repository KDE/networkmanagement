include_directories(
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs
   ${NETWORKMANAGER_INCLUDE_DIRS}
   ${NM-UTIL_INCLUDE_DIRS}
   ${CMAKE_CURRENT_SOURCE_DIR}/dbus
   ${CMAKE_CURRENT_BUILD_DIR}
)

#add_subdirectory(dbus)
#add_subdirectory(fakemanager)

#add_subdirectory(tests)

# TODO: add wimaxnetworkinterface.cpp, wimax-nsp.cpp
set(solid_networkmanager_PART_SRCS
   #NetworkManager-network.cpp
   #NetworkManager-networkmanager.cpp
   #NetworkManager-networkinterface.cpp
   #NetworkManager-wirelessnetwork.cpp
   #NetworkManager-dbushelper.cpp
   accesspoint.cpp
   manager.cpp
   networkinterface.cpp
   wirednetworkinterface.cpp
   wirelessnetworkinterface.cpp
   networkmodeminterface.cpp
   networkbtinterface.cpp
   nmpropertyhelper.cpp
)

# TODO: add dbus/nm-ip6-configinterface.cpp, dbus/nm-device-olpc-mesh.cpp
set( DBUS_INTERFACES_FILES
   dbus/activeconnection.cpp
   dbus/nm-managerinterface.cpp
   dbus/nm-deviceinterface.cpp
   dbus/nm-access-pointinterface.cpp
   dbus/nm-device-ethernetinterface.cpp
   dbus/nm-device-wifiinterface.cpp
   dbus/nm-device-wimaxinterface.cpp
   dbus/nm-device-modeminterface.cpp
   dbus/nm-device-btinterface.cpp
   dbus/nm-ip4-configinterface.cpp
   dbus/nm-dhcp4-configinterface.cpp
   dbus/nm-active-connectioninterface.cpp
)


kde4_add_library(solid_networkmanager09_static STATIC ${solid_networkmanager_PART_SRCS} ${DBUS_INTERFACES_FILES})
target_link_libraries(solid_networkmanager09_static ${KDE4_KDECORE_LIBS} solidcontrolnm09ifaces)

#kde4_add_plugin(solid_networkmanager ${solid_networkmanager_PART_SRCS})
kde4_add_plugin(solid_networkmanager09 module.cpp ${solid_networkmanager_PART_SRCS} ${DBUS_INTERFACES_FILES})

set(NMPROPTEST_SRCS nmpropertyhelper.cpp nmpropertytest.cpp)
#kde4_add_executable(nmpropertytest ${NMPROPTEST_SRCS})
#target_link_libraries(nmpropertytest ${KDE4_KDECORE_LIBS})

target_link_libraries(solid_networkmanager09 ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrolnm09 solidcontrolnm09ifaces)
install(TARGETS solid_networkmanager09 DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_networkmanager09.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )

kde4_install_icons( ${ICON_INSTALL_DIR} )
