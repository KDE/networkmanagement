project(testGeneral)

set(testGeneral_SRCS
 generalsettingstest.cpp
 ../networkprofile.cpp
 ../ifaceitemmodel.cpp
 ../generalsettingswidget.cpp
)

kde4_add_executable(testGeneral ${testGeneral_SRCS})

target_link_libraries(testGeneral  ${KDE4_KDEUI_LIBS} solidcontrol)
