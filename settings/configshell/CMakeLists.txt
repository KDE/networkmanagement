include_directories(../../libs)
include_directories(../../libs/internals)
include_directories(../../libs/ui)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libs/internals)

## next target: connection editor shell app for new connections

set(networkmanagement_configshell_SRCS
   main.cpp
)

kde4_add_executable(networkmanagement_configshell ${networkmanagement_configshell_SRCS})

target_link_libraries(networkmanagement_configshell knmui knmstorage solidcontrol ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})

install(TARGETS networkmanagement_configshell DESTINATION ${LIBEXEC_INSTALL_DIR})

