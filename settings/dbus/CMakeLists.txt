set( ADAPTER_INTROSPECTION_XML_FILES
   introspection/nm-settings.xml
   introspection/nm-settings-connection.xml
   )

set_source_files_properties( ${ADAPTER_INTROSPECTION_XML_FILES} PROPERTIES NO_NAMESPACE TRUE )

set( FILES_TO_BUILD activeconnection.cpp
   nm-manager-clientinterface.cpp
   nm-deviceinterface.cpp
   nm-access-pointinterface.cpp
   nm-device-802-3-ethernetinterface.cpp
   nm-device-802-11-wirelessinterface.cpp
   nm-ip4-configinterface.cpp
)

QT4_ADD_DBUS_ADAPTER( FILES_TO_BUILD ${ADAPTER_INTROSPECTION_XML_FILES} )

TARGET_LINK_LIBRARIES(networkmanagerdbus ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

