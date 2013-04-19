

########### next target ###############

set(solidcontrolnm09ifaces_LIB_SRCS
   modemcdmainterface.cpp
   modemgsmcontactsinterface.cpp
   modemgsmnetworkinterface.cpp
   modemgsmussdinterface.cpp
   modemlocationinterface.cpp
   modemmanagerinterface.cpp
   modemgsmcardinterface.cpp
   modemgsmhsointerface.cpp
   modemgsmsmsinterface.cpp
   modeminterface.cpp
   modemmanager.cpp
   networkinterface.cpp
   networkmodeminterface.cpp
   networkbtinterface.cpp
   networkmanager.cpp
   wirednetworkinterface.cpp
   wirelessaccesspoint.cpp
   wirelessnetworkinterface.cpp
   )

set(unused
   networkmanager.cpp
   network.cpp
   wirelessnetwork.cpp
   authentication.cpp
   )

kde4_add_library(solidcontrolnm09ifaces SHARED ${solidcontrolnm09ifaces_LIB_SRCS})

target_link_libraries(solidcontrolnm09ifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(solidcontrolnm09ifaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidcontrolnm09ifaces EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}  )


########### install files ###############

install( FILES
   modemcdmainterface.h
   modemgsmcontactsinterface.h
   modemgsmnetworkinterface.h
   modemgsmussdinterface.h
   modemlocationinterface.h
   modemmanagerinterface.h
   modemgsmcardinterface.h
   modemgsmhsointerface.h
   modemgsmsmsinterface.h
   modeminterface.h
   modemmanager.h
   networkmanager.h
   networkinterface.h
   networkmodeminterface.h
   networkbtinterface.h
   wirednetworkinterface.h
   wirelessnetworkinterface.h
   wirelessaccesspoint.h
   authentication.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/solid/controlnm09/ifaces COMPONENT Devel)

