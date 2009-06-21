include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_SOURCE_DIR}/../internals
   ${CMAKE_CURRENT_BINARY_DIR}/../internals
   )

add_subdirectory(tests)

set(knmservice_SRCS
   # defines interface for objects observing connections
   connectionhandler.cpp

   # list of all connections, maintains connectionhandlers
   connectionlist.cpp

   # loads/saves connections
   connectionlistpersistence.cpp

   # provides dbus configure() for config UI
   connectionlistpersistencedbus.cpp

   # list of things that may be connected: connections and unconfigured networks
   activatablelist.cpp

   # maintains NIC activatable providers
   networkinterfacemonitor.cpp

   # relates connections to network interfaces
   networkinterfaceactivatableprovider.cpp

   # interface for tracking activatable events
   activatableobserver.cpp

   # simple debug output observer
   activatabledebug.cpp
   )

kde4_add_library(knmservice SHARED ${knmservice_SRCS})

target_link_libraries(knmservice knminternals solidcontrol ${KDE4_KDECORE_LIBS})

set_target_properties(knmservice PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

install(TARGETS knmservice ${INSTALL_TARGETS_DEFAULT_ARGS})

