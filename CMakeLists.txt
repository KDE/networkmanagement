project(plasma-networkmanagement)

option(INSTALL_KNM_AUTOSTART "Install the autostart file for knetworkmanager. Warning: This prevents the plasmoid from working properly")
set (MINIMUM_NM_VERSION_REQUIRED "0.9.0")

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

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})
macro_optional_find_package(MobileBroadbandProviderInfo)
macro_log_feature(MOBILEBROADBANDPROVIDERINFO_FOUND "mobile-broadband-provider-info" "Database of mobile broadband service providers" "http://live.gnome.org/NetworkManager/MobileBroadband/ServiceProviders" FALSE "" "Needed for Mobile Connection Wizard support in Broadband Modem Management")

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${KDE4_INCLUDES})

#include(ConfigureChecks.cmake)

find_package(NetworkManager ${MINIMUM_NM_VERSION_REQUIRED})

if (NETWORKMANAGER_FOUND)
    find_package(GLIB2 REQUIRED)

    configure_file(backends/NetworkManager/config-nm09backend.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/backends/NetworkManager/config-nm09backend.h )

    if (${NETWORKMANAGER_VERSION} VERSION_GREATER ${MINIMUM_NM_VERSION_REQUIRED} OR
        ${NETWORKMANAGER_VERSION} VERSION_EQUAL ${MINIMUM_NM_VERSION_REQUIRED})
        set(NM_0_9 TRUE)
        add_definitions(-DMINIMUM_NM_VERSION_REQUIRED=\\"${MINIMUM_NM_VERSION_REQUIRED}\\")
        include_directories(
           ${NETWORKMANAGER_INCLUDE_DIRS}
           ${NM-UTIL_INCLUDE_DIRS}
           ${GLIB2_INCLUDE_DIR}
           ${CMAKE_CURRENT_BUILD_DIR}
           solidcontrolfuture/libs
           ${KDE4_INCLUDE_DIR}/solid/control
        )

        add_subdirectory(solidcontrolfuture)
        add_subdirectory(applet)
        add_subdirectory(backends)
        add_subdirectory(dataengine)
        add_subdirectory(libs)
        add_subdirectory(tests)
        add_subdirectory(settings)
        add_subdirectory(vpnplugins)
        add_subdirectory(kded)

        # This is unmaintained, do not use it.
        #add_subdirectory(monolithic)

        install( FILES networkmanagement.notifyrc DESTINATION ${DATA_INSTALL_DIR}/networkmanagement )

        if (DBUS_SYSTEM_POLICY_DIR)
            install( FILES NetworkManager-kde4.conf DESTINATION ${DBUS_SYSTEM_POLICY_DIR})
        else (DBUS_SYSTEM_POLICY_DIR)
            message(STATUS "It is necessary to specify the directory where the client policy file for NetworkManager-kde4 is installed as DBUS_SYSTEM_POLICY_DIR.  Normally this is /etc/dbus-1/system.d on a Linux system")
        endif (DBUS_SYSTEM_POLICY_DIR)
    else()
        message(FATAL_ERROR "ERROR: NetworkManager version '${NETWORKMANAGER_VERSION}' does not match minimum required (${MINIMUM_NM_VERSION_REQUIRED})")
    endif()
else(NETWORKMANAGER_FOUND)
    message(FATAL_ERROR "ERROR: NetworkManager headers >= ${MINIMUM_NM_VERSION_REQUIRED} not installed. http://projects.gnome.org/NetworkManager/")
endif(NETWORKMANAGER_FOUND)

if(INSTALL_KNM_AUTOSTART)
    message("Installing autostart file for knm, use cmake -DINSTALL_KNM_AUTOSTART=ON to switch it on. Note that this will break the plasmoid. For this to work, make sure the kded DBus service is NOT loaded.")
endif(INSTALL_KNM_AUTOSTART)

macro_display_feature_log()

