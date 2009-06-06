include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   )

# next target: dbus interfaces library
set(knmconnection_SRCS
    connectable.cpp
    connection.cpp
    wirelessconnection.cpp
    wirelessnetworkitem.cpp
    )

qt4_add_dbus_adaptor(
    knmconnection_SRCS
    interfaces/org.kde.networkmanagement.connectable.xml
    connectable.h
    Knm::Connectable
)

qt4_add_dbus_adaptor(
    knmconnection_SRCS
    interfaces/org.kde.networkmanagement.connection.xml
    connection.h
    Knm::Connection
)

qt4_add_dbus_adaptor(
    knmconnection_SRCS
    interfaces/org.kde.networkmanagement.wirelessconnection.xml
    wirelessconnection.h
    Knm::WirelessConnection
)

qt4_add_dbus_adaptor(
    knmconnection_SRCS
    interfaces/org.kde.networkmanagement.wirelessnetworkitem.xml
    wirelessnetworkitem.h
    Knm::WirelessNetworkItem
)

kde4_add_library(
    knmconnections ${knmconnection_SRCS}
)

target_link_libraries(knmconnections ${KDE4_KDECORE_LIBS} solidcontrol)

set_target_properties(knmconnections PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

install(TARGETS knmconnections ${INSTALL_TARGETS_DEFAULT_ARGS})


