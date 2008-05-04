project(testWireless)

set(testWireless_SRCS
 wirelesssettingstest.cpp
 ../apitemview.cpp
 ../apitemmodel.cpp
 ../accesspoint.cpp
 ../apitemdelegate.cpp
 ../encryptionsettingswidget.cpp
 ../wirelesssettingswidget.cpp
)

# search for packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(Perl REQUIRED)
find_package(ZLIB REQUIRED)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}
                     ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} )
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})


kde4_add_executable(testWireless ${testWireless_SRCS})

target_link_libraries(testWireless  ${KDE4_KDEUI_LIBS} solidcontrol)
