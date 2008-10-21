project(plasma-networkmanager)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include(MacroOptionalAddSubdirectory)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${KDE4_INCLUDES})

#include(ConfigureChecks.cmake)
find_package(Plasma REQUIRED)
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

add_subdirectory(applet)
add_subdirectory(libs)
add_subdirectory(tests)
add_subdirectory(settings)

## next target: networkmanager applet
## not built, the real applet is in applet/
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

#kde4_add_ui_files(networkmanager_SRCS widgets/scanwidget.ui)
#kde4_add_ui_files(networkmanager_SRCS widgets/generalsettingswidget.ui)
#kde4_add_ui_files(networkmanager_SRCS widgets/manageprofilewidget.ui)
#kde4_add_ui_files(networkmanager_SRCS widgets/manageprofilewidget.ui)
#kde4_add_ui_files(networkmanager_SRCS widgets/wirelesssettingswidget.ui)
#kde4_add_plugin(plasma_applet_networkmanager ${networkmanager_SRCS})
#target_link_libraries(plasma_applet_networkmanager plasma ${KDE4_KIO_LIBS} solidcontrol)

#install(TARGETS plasma_applet_networkmanager DESTINATION ${PLUGIN_INSTALL_DIR})
#install(FILES plasma-applet-networkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(FILES networkmanager.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/networkmanager/)
else(NM_0_7)
      macro_log_feature(NM_0_7 "NetworkManager ${NETWORKMANAGER_VERSION}" "NetworkManager is a daemon for user controlled networking" "http://www.gnome.org/projects/NetworkManager")
endif(NM_0_7)
endif(NETWORKMANAGER_FOUND)
