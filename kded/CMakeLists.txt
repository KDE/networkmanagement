include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/service
   # this will become optional
   ${CMAKE_CURRENT_SOURCE_DIR}/../backends/NetworkManager
   )
   ## next target: test service

set(kded_networkmanagement_SRCS
   service.cpp
   )

kde4_add_plugin(kded_networkmanagement ${kded_networkmanagement_SRCS})

target_link_libraries(kded_networkmanagement
   ${KDE4_KDECORE_LIBS}
   knminternals
   knmservice
   knm_nm
   )

install(TARGETS kded_networkmanagement DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES networkmanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)

