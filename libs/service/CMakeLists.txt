include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_SOURCE_DIR}/../internals
   ${CMAKE_CURRENT_BINARY_DIR}/../internals
   )

add_subdirectory(tests)

set(knmservice_SRCS
   connectionhandler.cpp
   connectionlist.cpp
   connectionlistpersistence.cpp
   connectablelist.cpp
   networkinterfacemonitor.cpp
   networkinterfaceconnectableprovider.cpp
   )

kde4_add_library(knmservice SHARED ${knmservice_SRCS})

target_link_libraries(knmservice knminternals solidcontrol ${KDE4_KDECORE_LIBS})

set_target_properties(knmservice PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

install(TARGETS knmservice ${INSTALL_TARGETS_DEFAULT_ARGS})

