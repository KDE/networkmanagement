include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/service
   ${CMAKE_CURRENT_SOURCE_DIR}/../backends/NetworkManager
   )

set(monolithic_SRCS
   monolithic.cpp
   simpleui.cpp
   )

kde4_add_executable(knetworkmanager ${monolithic_SRCS})

target_link_libraries(knetworkmanager
   ${KDE4_KDECORE_LIBS}
   ${KDE4_KDEUI_LIBS}
   knotificationitem-1
   knminternals
   knmservice
   knm_nm
   )

