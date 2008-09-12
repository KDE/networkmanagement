include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   )
## next target: ui library

# CMake needs one .cpp in the file list to guess the language.  Perhaps in cases like this the language can be manually set.
# Library is not used for now.
set(knmui_SRCS dummy.cpp)
kde4_add_ui_files(knmui_SRCS connection.ui wired.ui ipv4.ui 802-11-wireless.ui)
kde4_add_library(knmui SHARED ${knmui_SRCS})
target_link_libraries(knmui ${KDE4_KDEUI_LIBS})

set_target_properties(knmui PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )
install(TARGETS knmui ${INSTALL_TARGETS_DEFAULT_ARGS})


