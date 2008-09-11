project(plasma-networkmanager)
macro_optional_find_package(NetworkManager)
macro_log_feature(NETWORKMANAGER_FOUND "NetworkManager (any version)" "NetworkManager is a daemon for user controlled networking" "http://www.gnome.org/projects/NetworkManager")
if (NETWORKMANAGER_FOUND)
   macro_ensure_version( "0.7.0" ${NETWORKMANAGER_VERSION} NM_0_7)
   find_package(GLIB2 REQUIRED)
   if (NM_0_7)

include_directories(
   ${NETWORKMANAGER_INCLUDE_DIRS}
   ${NM-UTIL_INCLUDE_DIRS}
   ${GLIB2_INCLUDE_DIR}
   ${CMAKE_CURRENT_BUILD_DIR}
)

## target: networkmanager applet

set(networkmanager_SRCS
    networkmanager.cpp
    nmmenu.cpp
    widgets/ifaceitemmodel.cpp
    widgets/apitemview.cpp
    widgets/apitemmodel.cpp
    widgets/accesspoint.cpp
    widgets/apitemdelegate.cpp
    widgets/profileitemmodel.cpp
    widgets/networkprofile.cpp
    widgets/manageprofilewidget.cpp
    widgets/encryptionsettingswidget.cpp
    widgets/wirelesssettingswidget.cpp
    widgets/generalsettingswidget.cpp
    widgets/editprofilewidget.cpp
    widgets/scanwidget.cpp
)

kde4_add_ui_files(networkmanager_SRCS widgets/scanwidget.ui)
kde4_add_plugin(plasma_applet_networkmanager ${networkmanager_SRCS})
target_link_libraries(plasma_applet_networkmanager plasma ${KDE4_KIO_LIBS} solidcontrol)

## next target: test for ConfigXml to make sure it can handle our connections

set(testconfigxml_SRCS
   testconfigxml.cpp
   secretstoragehelper.cpp
   settings/configxml.cpp
)

kde4_add_executable(testconfigxml ${testconfigxml_SRCS})
target_link_libraries(testconfigxml ${KDE4_KDEUI_LIBS})

## next target: test service

set(testservice_SRCS
   secretstoragehelper.cpp
   testsettingsservice.cpp
   settings/connection.cpp
   settings/kconfigtoservice.cpp
   settings/networksettings.cpp
   settings/configxml.cpp
   )

kde4_add_kcfg_files(testservice_SRCS settings/networksettingsprefs.kcfgc)

message(STATUS "testservice_SRCS is ${testservice_SRCS}")
kde4_add_ui_files(testservice_SRCS settings/connection.ui settings/wired.ui settings/ipv4.ui)
kde4_add_executable(testservice ${testservice_SRCS})
target_link_libraries(testservice ${KDE4_KUTILS_LIBS} solidcontrol)

## next target: ui library

# CMake needs one .cpp in the file list to guess the language.  Perhaps in cases like this the language can be manually set.
# Library is not used for now.
set(knmui_SRCS dummy.cpp)
#kde4_add_ui_files(knmui_SRCS settings/connection.ui settings/wired.ui settings/ipv4.ui)
#kde4_add_library(knmui SHARED ${knmui_SRCS})
#target_link_libraries(knmui ${KDE4_KDEUI_LIBS})

#set_target_properties(knmui PROPERTIES
#   VERSION ${GENERIC_LIB_VERSION}
#   SOVERSION ${GENERIC_LIB_SOVERSION}
#   )
#install(TARGETS knmui ${INSTALL_TARGETS_DEFAULT_ARGS})

## next target: wired KCM

set(kcm_knetworkmanager_wired_SRCS
   # UI logic
   settings/wiredpreferences.cpp
   secretstoragehelper.cpp
   settings/configxml.cpp)
kde4_add_ui_files(kcm_knetworkmanager_wired_SRCS settings/connection.ui settings/wired.ui settings/ipv4.ui)
kde4_add_plugin(kcm_knetworkmanager_wired ${kcm_knetworkmanager_wired_SRCS})

target_link_libraries(kcm_knetworkmanager_wired ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY})

install(TARGETS kcm_knetworkmanager_wired DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_knetworkmanager_wired.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)

##

install(TARGETS plasma_applet_networkmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-networkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES networkmanager.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/)
install(FILES settings/802-11-wireless.kcfg
   settings/802-3-ethernet.kcfg
   settings/gsm.kcfg
   settings/ppp.kcfg
   settings/vpn.kcfg
   settings/802-11-wireless-security.kcfg
   settings/cdma.kcfg
   settings/ipv4.kcfg
   settings/pppoe.kcfg
   settings/802-1x.kcfg
   settings/connection.kcfg
   settings/serial.kcfg
   DESTINATION ${DATA_INSTALL_DIR}/knetworkmanager/schemas/)

else(NM_0_7)
      macro_log_feature(NM_0_7 "NetworkManager ${NETWORKMANAGER_VERSION}" "NetworkManager is a daemon for user controlled networking" "http://www.gnome.org/projects/NetworkManager")
endif(NM_0_7)
endif(NETWORKMANAGER_FOUND)
