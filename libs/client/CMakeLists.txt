add_subdirectory(tests)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_SOURCE_DIR}/../internals
   ${CMAKE_CURRENT_BINARY_DIR}
   )

set(knmclient_SRCS
   remoteactivatable.cpp
   remoteinterfaceconnection.cpp
   remotewirelessinterfaceconnection.cpp
   remotewirelessnetworkitem.cpp
   remoteactivatablelist.cpp
)

set_source_files_properties( ../service/interfaces/org.kde.networkmanagement.xml PROPERTIES NO_NAMESPACE TRUE CLASSNAME NetworkManagementInterface)

QT4_ADD_DBUS_INTERFACE(knmclient_SRCS
   ../service/interfaces/org.kde.networkmanagement.xml
   networkmanagementinterface
   )

set_source_files_properties( ../service/interfaces/org.kde.networkmanagement.activatable.xml PROPERTIES NO_NAMESPACE TRUE CLASSNAME ActivatableInterface)

QT4_ADD_DBUS_INTERFACE(knmclient_SRCS
   ../service/interfaces/org.kde.networkmanagement.activatable.xml
   activatableinterface
   )

set_source_files_properties( ../service/interfaces/org.kde.networkmanagement.interfaceconnection.xml PROPERTIES NO_NAMESPACE TRUE CLASSNAME InterfaceConnectionInterface)

QT4_ADD_DBUS_INTERFACE(knmclient_SRCS
   ../service/interfaces/org.kde.networkmanagement.interfaceconnection.xml
   interfaceconnectioninterface
   )

set_source_files_properties( ../service/interfaces/org.kde.networkmanagement.wirelessinterfaceconnection.xml PROPERTIES NO_NAMESPACE TRUE CLASSNAME WirelessInterfaceConnectionInterface)

QT4_ADD_DBUS_INTERFACE(knmclient_SRCS
   ../service/interfaces/org.kde.networkmanagement.wirelessinterfaceconnection.xml
   wirelessinterfaceconnectioninterface
   )

set_source_files_properties( ../service/interfaces/org.kde.networkmanagement.wirelessnetworkitem.xml PROPERTIES NO_NAMESPACE TRUE CLASSNAME WirelessNetworkItemInterface)

QT4_ADD_DBUS_INTERFACE(knmclient_SRCS
   ../service/interfaces/org.kde.networkmanagement.wirelessnetworkitem.xml
   wirelessnetworkiteminterface
   )

kde4_add_library(knmclient SHARED ${knmclient_SRCS})

target_link_libraries(knmclient ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${KDE4_KDECORE_LIBS})

set_target_properties(knmclient PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

install(TARGETS knmclient ${INSTALL_TARGETS_DEFAULT_ARGS})



