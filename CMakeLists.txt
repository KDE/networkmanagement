project(plasma-networkmanagement)

set (MINIMUM_NM_VERSION_REQUIRED "0.9.4")

# We need some features from QML components from kde-runtime >= 4.9.0, like
# TextField.qml's accepted() signal.
find_package(KDE4 "4.9.0" REQUIRED)
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

pkg_check_modules(MODEMMANAGERQT REQUIRED ModemManagerQt)
if (NOT MODEMMANAGERQT_FOUND)
    message(FATAL_ERROR "ERROR: ModemManagerQt not found. http://projects.kde.org/search?q=modemmanager&scope=all&all_words=1&projects=1&submit=Submit")
endif (NOT MODEMMANAGERQT_FOUND)

pkg_check_modules(NETWORKMANAGERQT REQUIRED NetworkManagerQt)
if (NOT NETWORKMANAGERQT_FOUND)
    message(FATAL_ERROR "ERROR: NetworkManagerQt not found. http://projects.kde.org/search?q=networkmanager&scope=all&all_words=1&projects=1&submit=Submit")
endif (NOT NETWORKMANAGERQT_FOUND)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES} ${MODEMMANAGERQT_INCLUDE_DIRS} ${NETWORKMANAGERQT_INCLUDE_DIRS} ${CMAKE_INCLUDE_PATH})
link_directories(${MODEMMANAGERQT_LIBRARY_DIRS} ${NETWORKMANAGERQT_LIBRARY_DIRS} ${CMAKE_LIBRARY_PATH})

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
           ${KDE4_INCLUDE_DIR}/solid/control
        )

        add_subdirectory(applet)
        add_subdirectory(backends)
        add_subdirectory(dataengine)
        add_subdirectory(libs)
        add_subdirectory(tests)
        add_subdirectory(settings)
        add_subdirectory(vpnplugins)
        add_subdirectory(kded)

        install( FILES networkmanagement.notifyrc DESTINATION ${DATA_INSTALL_DIR}/networkmanagement )
    else()
        message(FATAL_ERROR "ERROR: NetworkManager version '${NETWORKMANAGER_VERSION}' does not match minimum required (${MINIMUM_NM_VERSION_REQUIRED})")
    endif()
else(NETWORKMANAGER_FOUND)
    message(FATAL_ERROR "ERROR: NetworkManager headers >= ${MINIMUM_NM_VERSION_REQUIRED} not installed. http://projects.gnome.org/NetworkManager/")
endif(NETWORKMANAGER_FOUND)

macro_display_feature_log()

