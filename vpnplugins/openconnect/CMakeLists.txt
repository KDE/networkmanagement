set (MINIMUM_OPENCONNECT_VERSION_REQUIRED "3.99")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

macro_optional_find_package(OpenConnect ${MINIMUM_OPENCONNECT_VERSION_REQUIRED})
if (OPENCONNECT_FOUND)
    if (${OPENCONNECT_VERSION} VERSION_LESS "3.99")
        macro_optional_find_package(OpenSSL)
        macro_log_feature(OPENSSL_FOUND "OpenSSL headers" "Encryption suite" "http://www.openssl.org" FALSE "" "Needed for OpenConnect support in Network Management")
        if (OPENSSL_FOUND)
            set (MINIMUM_OPENCONNECT_VERSION_REQUIRED "3.03")
            set (maybe_OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES})
        endif()
    endif()

    if (${OPENCONNECT_VERSION} VERSION_GREATER ${MINIMUM_OPENCONNECT_VERSION_REQUIRED} OR
        ${OPENCONNECT_VERSION} VERSION_EQUAL ${MINIMUM_OPENCONNECT_VERSION_REQUIRED})
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/ui)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/internals)

        set(openconnect_SRCS
        openconnectui.cpp
        openconnectwidget.cpp
        openconnectauth.cpp
        openconnectauthworkerthread.cpp
        )

        kde4_add_ui_files(openconnect_SRCS openconnectprop.ui openconnectauth.ui)

        kde4_add_plugin(networkmanagement_openconnectui ${openconnect_SRCS})

        target_link_libraries(networkmanagement_openconnectui ${KDE4_KIO_LIBS} knminternals knmui ${OPENCONNECT_LIBRARIES} ${maybe_OPENSSL_LIBRARIES})
        install(TARGETS networkmanagement_openconnectui  DESTINATION ${PLUGIN_INSTALL_DIR})

        install( FILES networkmanagement_openconnectui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
    else()
        message("ERROR: OpenConnection version '${OPENCONNECT_VERSION}' does not match minimum required (${MINIMUM_OPENCONNECT_VERSION_REQUIRED})")
        message("OpenConnect plugin will not be built")
        set(OPENCONNECT_FOUND False)
    endif()
else (OPENCONNECT_FOUND)
    message("OpenConnect plugin will not be built")
endif (OPENCONNECT_FOUND)

macro_log_feature(OPENCONNECT_FOUND "OpenConnect headers and library" "Cisco AnyConnect compatible VPN client" "http://www.infradead.org/openconnect.html" FALSE ${MINIMUM_OPENCONNECT_VERSION_REQUIRED} "Needed for OpenConnect support in Network Management")
