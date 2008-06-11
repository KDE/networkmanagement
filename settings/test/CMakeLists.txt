project(settingstest)

set(settings_SRCS
 settingsdaemontest.cpp
)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}
                        ${KDE4_INCLUDE_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} /usr/include/NetworkManager/)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})

kde4_add_executable(settingsdaemontest ${settings_SRCS})

target_link_libraries(settingsdaemontest ${KDE4_KDEUI_LIBS} ${QT_QTDBUS_LIBRARY})
