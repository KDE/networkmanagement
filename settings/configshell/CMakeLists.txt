add_definitions(-DKDE_DEFAULT_DEBUG_AREA=51013)

include_directories(../../libs)
include_directories(../../libs/internals)
include_directories(../../libs/service)
include_directories(../../libs/ui)
include_directories(../../backends/NetworkManager)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libs/internals)

## next target: connection editor shell app for new connections

set(networkmanagement_configshell_SRCS
   main.cpp
   bluetooth.cpp
   manageconnection.cpp
   ../config/mobileconnectionwizard.cpp
   ../config/mobileproviders.cpp
)

kde4_add_executable(networkmanagement_configshell ${networkmanagement_configshell_SRCS})

target_link_libraries(networkmanagement_configshell knmservice knm_nm knmui knminternals solidcontrolnm09 ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY})
target_link_libraries(networkmanagement_configshell ${KDE4_SOLID_LIBS} ${KDE4_KCMUTILS_LIBS})

install(TARGETS networkmanagement_configshell DESTINATION ${LIBEXEC_INSTALL_DIR})

