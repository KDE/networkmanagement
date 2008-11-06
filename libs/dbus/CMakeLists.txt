include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   )

# next target: dbus interfaces library
set(knmdbus_SRCS
   nm-active-connectioninterface.cpp
   nm-exported-connectioninterface.cpp
   nm-settingsinterface.cpp
   networkmanagersettings.cpp
   remoteconnection.cpp
   )

kde4_add_library(knmdbus SHARED ${knmdbus_SRCS})
target_link_libraries(knmdbus ${KDE4_KDECORE_LIBS} solidcontrol)

set_target_properties(knmdbus PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )

install(TARGETS knmdbus ${INSTALL_TARGETS_DEFAULT_ARGS})


