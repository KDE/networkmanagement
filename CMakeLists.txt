project(plasma-networkmanager)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include(MacroOptionalAddSubdirectory)
include(TestBigEndian)

TEST_BIG_ENDIAN(BIGENDIAN)
if (BIGENDIAN)
#can't just place WORDS_BIGENDIAN in TEST_BIGENDIAN as our code uses #ifndef
   set(WORDS_BIGENDIAN 1)
endif (BIGENDIAN)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${KDE4_INCLUDES})

#include(ConfigureChecks.cmake)

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
#disabled pending porting
#add_subdirectory(vpnplugins)

install( FILES knetworkmanager.notifyrc DESTINATION ${DATA_INSTALL_DIR}/knetworkmanager )

if (DBUS_SYSTEM_POLICY_DIR)
install( FILES NetworkManager-kde4.conf DESTINATION ${DBUS_SYSTEM_POLICY_DIR})
else (DBUS_SYSTEM_POLICY_DIR)
message(SEND_ERROR "It is necessary to specify the directory where the client policy file for NetworkManager-kde4 is installed as DBUS_SYSTEM_POLICY_DIR.  Normally this is /etc/dbus-1/system.d on a Linux system")
endif (DBUS_SYSTEM_POLICY_DIR)

else(NM_0_7)
      macro_log_feature(NM_0_7 "NetworkManager ${NETWORKMANAGER_VERSION}" "NetworkManager is a daemon for user controlled networking" "http://www.gnome.org/projects/NetworkManager")
endif(NM_0_7)
endif(NETWORKMANAGER_FOUND)
