include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/internals
   ${CMAKE_CURRENT_SOURCE_DIR}/../libs/service
   )

set(monolithic_SRCS
   monolithic.cpp
   )

kde4_add_executable(knetworkmanager ${monolithic_SRCS})

target_link_libraries(knetworkmanager
   ${KDE4_KDECORE_LIBS}
   ${KDE4_KDEUI_LIBS}
   knminternals
   knmservice
   )

