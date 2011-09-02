kde4_no_enable_final(solid)

include_directories( ${CMAKE_SOURCE_DIR}/workspace/libs )

add_subdirectory( ifaces )
#add_subdirectory( backends )
#add_subdirectory( tests )

########### next target ###############

set(solidcontrolnm09_LIB_SRCS
   frontendobject.cpp
   managerbase.cpp
   networking.cpp
   networkmanager.cpp
   networkinterface.cpp
   networkmodeminterface.cpp
   networkbtinterface.cpp
   networkipv4config.cpp
   wirednetworkinterface.cpp
   wirelessaccesspoint.cpp
   wirelessnetworkinterface.cpp
)

qt4_add_dbus_interface(solidcontrolnm09_LIB_SRCS org.kde.Solid.Networking.xml org_kde_solid_networking)

kde4_add_library(solidcontrolnm09 SHARED ${solidcontrolnm09_LIB_SRCS})

target_link_libraries(solidcontrolnm09 ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTNETWORK_LIBRARY} solidcontrolnm09ifaces ${QT_QTGUI_LIBRARY} )

set_target_properties(solidcontrolnm09 PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidcontrolnm09 EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES solidnetworkmanagernm09.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR}  )

install( FILES networkmanager.h networkinterface.h 
    wirednetworkinterface.h wirelessnetworkinterface.h
    wirelessaccesspoint.h singletondefs.h networking.h networkipv4confignm09.h solid_control_export.h 
    networkmodeminterface.h networkbtinterface.h
    DESTINATION
${INCLUDE_INSTALL_DIR}/solid/controlnm09 COMPONENT Devel)

