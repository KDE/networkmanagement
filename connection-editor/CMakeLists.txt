project(connectioneditor)
cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)

include(FindPkgConfig)

pkg_check_modules(QTNETWORKMANAGER REQUIRED QtNetworkManager)
if (NOT QTNETWORKMANAGER_FOUND)
    message(FATAL_ERROR "ERROR: QtNetworkManager not found. http://projects.kde.org/libnm-qt")
endif (NOT QTNETWORKMANAGER_FOUND)

pkg_check_modules(QTNETWORK REQUIRED QtNetwork)
pkg_check_modules(NETWORKMANAGER REQUIRED NetworkManager)

include_directories(${CMAKE_SOURCE_DIR}
		    ${CMAKE_BINARY_DIR}
		    ${QTNETWORK_INCLUDE_DIRS}
		    ${QTNETWORKMANAGER_INCLUDE_DIRS}
		    ${KDE4_INCLUDES})

link_directories(${QTNETWORK_LIBRARY_DIRS} ${QTNETWORKMANAGER_LIBRARY_DIRS})

set(connectioneditor_SRCS
   main.cpp
   connectioneditor.cpp
   connectionitem.cpp
   connectiontypeitem.cpp
   connectiondetaileditor.cpp
)

set(connectioneditor_HDRS
   connectioneditor.h
   connectionitem.h
   connectiontypeitem.h
   connectiondetaileditor.h
)

kde4_add_ui_files(connectioneditor_SRCS ui/connectioneditor.ui ui/connectiondetaileditor.ui)
qt4_wrap_cpp(connectioneditor_HDRS_MOC ${connectioneditor_HDRS})

add_executable(connectioneditor ${connectioneditor_SRCS} ${connectioneditor_HDRS_MOC})
target_link_libraries(connectioneditor QtNetworkManager ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${KDE4_KDEUI_LIBS})

