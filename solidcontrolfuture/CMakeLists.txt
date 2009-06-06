set(solidcontrolfuture_LIB_SRCS
   wirelessnetworkinterfaceenvironment.cpp
   wirelessnetwork.cpp
   )

kde4_add_library(solidcontrolfuture SHARED ${solidcontrolfuture_LIB_SRCS})

target_link_libraries(solidcontrolfuture ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTNETWORK_LIBRARY} solidcontrol)

install(TARGETS solidcontrolfuture EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES wirelessnetwork.h
   DESTINATION
   ${INCLUDE_INSTALL_DIR}/solid/control COMPONENT Devel)
 
