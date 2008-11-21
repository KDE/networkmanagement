include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   )

## next target: storage layer library

set(knmstorage_SRCS
   secretstoragehelper.cpp
   configxml.cpp
)

kde4_add_kcfg_files(knmstorage_SRCS knmserviceprefs.kcfgc)

kde4_add_library(knmstorage SHARED ${knmstorage_SRCS})

target_link_libraries(knmstorage ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY})

set_target_properties(knmstorage PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
   )
install(TARGETS knmstorage ${INSTALL_TARGETS_DEFAULT_ARGS})


