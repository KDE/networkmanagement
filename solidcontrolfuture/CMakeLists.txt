# build solidcontrolnm09
add_subdirectory(libs)
macro_optional_add_subdirectory(solid)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include(MacroOptionalAddSubdirectory)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set(solidcontrolfuture_LIB_SRCS
   wirelessnetworkinterfaceenvironment.cpp
   wirelessnetwork.cpp
   networkipv6config.cpp
   )

kde4_add_library(solidcontrolfuture SHARED ${solidcontrolfuture_LIB_SRCS})

target_link_libraries(solidcontrolfuture ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTNETWORK_LIBRARY} solidcontrolnm09)

install(TARGETS solidcontrolfuture EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

#install( FILES wirelessnetwork.h
#               wirelessnetworkinterfaceenvironment.h
#   DESTINATION
#   ${INCLUDE_INSTALL_DIR}/solid/control COMPONENT Devel)
 
