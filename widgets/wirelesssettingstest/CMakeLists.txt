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

kde4_add_ui_files(../scanwidget.ui ${testWireless_SRCS})
kde4_add_executable(testWireless ${testWireless_SRCS})

target_link_libraries(testWireless  ${KDE4_KDEUI_LIBS} solidcontrol)
