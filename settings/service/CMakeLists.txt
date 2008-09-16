include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs
   ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage
   )
   ## next target: test service

set(testservice_SRCS
   service.cpp
   connection.cpp
   kconfigtoservice.cpp
   networksettings.cpp
   )

kde4_add_kcfg_files(testservice_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/storage/knmserviceprefs.kcfgc)

qt4_add_dbus_adaptor(testservice_SRCS org.kde.knetworkmanagerd.xml kconfigtoservice.h KConfigToService knetworkmanagerserviceadaptor KNetworkManagerServiceAdaptor )

kde4_add_executable(testservice ${testservice_SRCS})
target_link_libraries(testservice knmstorage ${KDE4_KUTILS_LIBS} solidcontrol)


