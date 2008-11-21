include_directories(../../libs)
include_directories(../../libs/storage)
include_directories(../../libs/ui)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libs/storage)

## next target: connection editor shell app for new connections

set(knetworkmanager_configshell_SRCS
   main.cpp
)

kde4_add_executable(knetworkmanager_configshell ${knetworkmanager_configshell_SRCS})

target_link_libraries(knetworkmanager_configshell knmui knmstorage solidcontrol ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})

install(TARGETS knetworkmanager_configshell DESTINATION ${LIBEXEC_INSTALL_DIR})

