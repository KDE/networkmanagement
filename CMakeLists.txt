project(plasma-networkmanagement)

option(INSTALL_KNM_AUTOSTART "Install the autostart file for knetworkmanager. Warning: This prevents the plasmoid from working properly")


find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include(MacroOptionalAddSubdirectory)
include(TestBigEndian)

#indicate endianness to hashing functions
TEST_BIG_ENDIAN(BIGENDIAN)
if (BIGENDIAN)
#can't just place WORDS_BIGENDIAN in TEST_BIGENDIAN as our code uses #ifndef
   set(WORDS_BIGENDIAN 1)
endif (BIGENDIAN)

find_file(KDE4_MM_BACKEND_FOUND solid/control/modeminterface.h ${KDE4_INCLUDE_DIR} NO_DEFAULT_PATH)
if (KDE4_MM_BACKEND_FOUND)
	message("Compiling Modem Manager support.")
	add_definitions(-DCOMPILE_MODEM_MANAGER_SUPPORT=1)
endif (KDE4_MM_BACKEND_FOUND)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${KDE4_INCLUDES})

#include(ConfigureChecks.cmake)

macro_optional_find_package(NetworkManager)

macro_log_feature(NETWORKMANAGER_FOUND "NetworkManager headers" "User controllable networking" "http://projects.gnome.org/NetworkManager" FALSE "0.7" "Needed for NetworkManager support in Network Management")
if (NETWORKMANAGER_FOUND)
macro_ensure_version( "0.7.0" ${NETWORKMANAGER_VERSION} NM_0_7)
macro_ensure_version( "0.7.1" ${NETWORKMANAGER_VERSION} NM_0_7_1)
find_package(GLIB2 REQUIRED)

configure_file(backends/NetworkManager/config-nm07backend.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/backends/NetworkManager/config-nm07backend.h )

if (NM_0_7)

include_directories(
   ${NETWORKMANAGER_INCLUDE_DIRS}
   ${NM-UTIL_INCLUDE_DIRS}
   ${GLIB2_INCLUDE_DIR}
   ${CMAKE_CURRENT_BUILD_DIR}
)

add_subdirectory(applet)
add_subdirectory(backends)
add_subdirectory(libs)
add_subdirectory(tests)
add_subdirectory(settings)
add_subdirectory(vpnplugins)
add_subdirectory(solidcontrolfuture)
add_subdirectory(monolithic)
add_subdirectory(kded)

install( FILES networkmanagement.notifyrc DESTINATION ${DATA_INSTALL_DIR}/networkmanagement )

if (DBUS_SYSTEM_POLICY_DIR)
install( FILES NetworkManager-kde4.conf DESTINATION ${DBUS_SYSTEM_POLICY_DIR})
else (DBUS_SYSTEM_POLICY_DIR)
message(STATUS "It is necessary to specify the directory where the client policy file for NetworkManager-kde4 is installed as DBUS_SYSTEM_POLICY_DIR.  Normally this is /etc/dbus-1/system.d on a Linux system")
endif (DBUS_SYSTEM_POLICY_DIR)

else(NM_0_7)
      macro_log_feature(NM_0_7 "NetworkManager ${NETWORKMANAGER_VERSION}" "NetworkManager is a daemon for user controlled networking" "http://www.gnome.org/projects/NetworkManager")
endif(NM_0_7)
endif(NETWORKMANAGER_FOUND)

if(INSTALL_KNM_AUTOSTART)
	message("Installing autostart file for knm, use cmake -DINSTALL_KNM_AUTOSTART=ON to switch it on. Note that this will break the plasmoid. For this to work, make sure the kded DBus service is NOT loaded.")
endif(INSTALL_KNM_AUTOSTART)

macro_display_feature_log()

