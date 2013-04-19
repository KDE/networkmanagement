add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51011)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_SOURCE_DIR}/../internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../../solidcontrolfuture
   ${CMAKE_CURRENT_BINARY_DIR}/../internals
   )

# Disabled until someone updates them to use the new NM-0.9 code
#add_subdirectory(tests)

set(knmservice_SRCS
   # defines interface for objects observing connections
   connectionhandler.cpp

   # list of all connections, maintains connectionhandlers
   connectionlist.cpp

   # handles storage of secrets
   secretstorage.cpp

   #defines interface for objects storing secrets
   secretsprovider.cpp

   # provides dbus configure() for config UI
   #connectionlistpersistencedbus.cpp

   # list of things that may be connected: connections and unconfigured networks
   activatablelist.cpp

   # maintains NIC activatable providers
   networkinterfacemonitor.cpp

   # relates connections to network interfaces
   networkinterfaceactivatableprovider.cpp

   # subclass of the above, uses ethernet carrier status to select activatables
   wirednetworkinterfaceactivatableprovider.cpp

   # complex class for managing connections and unconfigured networks for wireless interfaces
   wirelessnetworkinterfaceactivatableprovider.cpp

   # relates gsm connections to network interfaces
   gsmnetworkinterfaceactivatableprovider.cpp

   # interface for tracking activatable events
   activatableobserver.cpp

   # simple debug output observer
   activatabledebug.cpp

   # configures unconfigured devices and new wireless networks on demand and connects them
   configurationlauncher.cpp

   # updates connection state as they are used
   connectionusagemonitor.cpp

   # exports activatables on the session bus to Plasmoids etc
   sessionabstractedservice.cpp

   # creates vpn activatables
   vpninterfaceconnectionprovider.cpp

   # performs KNotifications on events
   notificationmanager.cpp

   # static global definitions of event string ids
   events.cpp

   # sort logic for activatables
   sortedactivatablelist.cpp

   # dialog to ask for PIN code
   pindialog.cpp
   )

kde4_add_ui_files(knmservice_SRCS
   # Ask for PIN unlock code
   pinwidget.ui
)

qt4_add_dbus_adaptor(
    knmservice_SRCS
    interfaces/org.kde.networkmanagement.activatable.xml
    activatable.h
    Knm::Activatable
    activatableadaptor
    ActivatableAdaptor
)

qt4_add_dbus_adaptor(
    knmservice_SRCS
    interfaces/org.kde.networkmanagement.interfaceconnection.xml
    interfaceconnection.h
    Knm::InterfaceConnection
    interfaceconnectionadaptor
    InterfaceConnectionAdaptor
)

qt4_add_dbus_adaptor(
    knmservice_SRCS
    interfaces/org.kde.networkmanagement.wirelessinterfaceconnection.xml
    wirelessinterfaceconnection.h
    Knm::WirelessInterfaceConnection
    wirelessinterfaceconnectionadaptor
    WirelessInterfaceConnectionAdaptor
)

qt4_add_dbus_adaptor(
    knmservice_SRCS
    interfaces/org.kde.networkmanagement.wirelessnetwork.xml
    wirelessnetwork.h
    Knm::WirelessNetwork
    wirelessnetworkadaptor
    WirelessNetworkAdaptor
)

qt4_add_dbus_adaptor(
    knmservice_SRCS
    interfaces/org.kde.networkmanagement.gsminterfaceconnection.xml
    gsminterfaceconnection.h
    Knm::GsmInterfaceConnection
    gsminterfaceconnectionadaptor
    GsmInterfaceConnectionAdaptor
)

qt4_add_dbus_adaptor(knmservice_SRCS
   interfaces/org.kde.networkmanagement.xml
   sessionabstractedservice.h
   SessionAbstractedService
   sessionabstractedserviceadaptor
   SessionAbstractedServiceAdaptor
   )

kde4_add_library(knmservice SHARED ${knmservice_SRCS})

target_link_libraries(knmservice knminternals knmui solidcontrolnm09 solidcontrolfuture ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS}
   )

set_target_properties(knmservice PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

install(TARGETS knmservice ${INSTALL_TARGETS_DEFAULT_ARGS})

