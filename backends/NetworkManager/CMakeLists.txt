add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51020)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals/settings
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/service
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui
   ${CMAKE_CURRENT_SOURCE_DIR}/settings
   ${CMAKE_CURRENT_BINARY_DIR}
   )

## next target: internals layer library

set(knm_nm_LIB_SRCS
   connectiondbus.cpp

   settingdbus.cpp

   pbkdf2.cpp
   wephash.cpp

   settings/802-11-wireless-securitydbus.cpp

   settings/802-11-wirelessdbus.cpp

   settings/802-3-ethernetdbus.cpp

   settings/802-1xdbus.cpp

   settings/gsmdbus.cpp

   settings/cdmadbus.cpp

   settings/ipv4dbus.cpp

   settings/pppdbus.cpp

   settings/pppoedbus.cpp

   settings/serialdbus.cpp

   settings/vpndbus.cpp

   nmdbussettingsservice.cpp

   busconnection.cpp

   #org.freedesktop.NetworkManagerSettings.Connection
   exportedconnection.cpp
   
   #org.freedesktop.NetworkManagerSettings.Connection.Secrets
   exportedconnectionsecrets.cpp

   nmdbusactiveconnectionmonitor.cpp

   nm-active-connectioninterface.cpp

   nmdbussettingsconnectionprovider.cpp

   remoteconnection.cpp
   nm-settingsinterface.cpp
   nm-exported-connectioninterface.cpp
)

kde4_add_library(knm_nm SHARED ${knm_nm_LIB_SRCS})

target_link_libraries(knm_nm ${KDE4_KDEUI_LIBS} solidcontrol knminternals knmservice knmui ${QT_QTNETWORK_LIBRARY})


#set_target_properties(nmbackend PROPERTIES
#   VERSION ${GENERIC_LIB_VERSION}
#   SOVERSION ${GENERIC_LIB_SOVERSION}
#   )
install(TARGETS knm_nm ${INSTALL_TARGETS_DEFAULT_ARGS})

