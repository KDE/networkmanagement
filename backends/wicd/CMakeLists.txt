add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51021)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals/settings
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/service
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui
   ${CMAKE_CURRENT_BINARY_DIR}
   )


set(knm_wicd_LIB_SRCS
    wicdconnectionhandler.cpp
)

kde4_add_library(knm_wicd SHARED ${knm_wicd_LIB_SRCS})

target_link_libraries(knm_wicd ${KDE4_KDEUI_LIBS} knminternals knmservice knmui ${QT_QTNETWORK_LIBRARY})


#set_target_properties(nmbackend PROPERTIES
#   VERSION ${GENERIC_LIB_VERSION}
#   SOVERSION ${GENERIC_LIB_SOVERSION}
#   )
install(TARGETS knm_wicd ${INSTALL_TARGETS_DEFAULT_ARGS})

