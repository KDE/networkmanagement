project(settingstest)

set(settingsdaemon_SRCS
 settingsdaemontest.cpp
 ../networksettings.cpp
 ../connection.cpp
 ../wiredconnectionsetting.cpp
 ../wirelessconnectionsetting.cpp
)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

QT4_ADD_DBUS_ADAPTOR( settingsdaemon_SRCS ../dbus/introspection/nm-settings.xml ../networksettings.h NetworkSettings )
QT4_ADD_DBUS_ADAPTOR( settingsdaemon_SRCS ../dbus/introspection/nm-settings-connection.xml ../connection.h Connection )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}
                        ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} /usr/include/NetworkManager/)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})

kde4_add_executable(settingsdaemontest ${settingsdaemon_SRCS})
target_link_libraries(settingsdaemontest ${KDE4_KDEUI_LIBS} ${QT_QTDBUS_LIBRARY})

#client test
set(settingsclient_SRCS
 settingsclienttest.cpp
 client.cpp
 ../dbus/connectioninterface.cpp
)

set( INTERFACE_INTROSPECTION_XML_FILES
 ../dbus/introspection/nm-settings-connection.xml
)

set_source_files_properties( ${INTERFACE_INTROSPECTION_XML_FILES} PROPERTIES NO_NAMESPACE TRUE )
#QT4_ADD_DBUS_INTERFACE( settingsclient_SRCS ../dbus/introspection/nm-settings-connection.xml connectioninterface)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_SOURCE_DIR}/../dbus ${CMAKE_CURRENT_BINARY_DIR}
                        ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} /usr/include/NetworkManager/)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})

kde4_add_executable(settingsclienttest ${settingsclient_SRCS} )
target_link_libraries(settingsclienttest ${KDE4_KDEUI_LIBS} ${QT_QTDBUS_LIBRARY})
