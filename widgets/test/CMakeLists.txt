project(testAddProfile)

set(testAddProfile_SRCS
 commontest.cpp
 ../addprofilewidget.cpp
 ../ifaceitemmodel.cpp
 ../configifacewidget.cpp
 ../apitemview.cpp
 ../apitemmodel.cpp
 ../accesspoint.cpp
 ../apitemdelegate.cpp
 ../encryptionsettingswidget.cpp
 ../wirelesssettingswidget.cpp
)

kde4_add_executable(testAddProfile ${testAddProfile_SRCS})

target_link_libraries(testAddProfile  ${KDE4_KDEUI_LIBS} solidcontrol)
