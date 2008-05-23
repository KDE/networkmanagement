project(testManagedProfiles)

set(testManageProfiles_SRCS
 manageprofiles.cpp
 ../ifaceitemmodel.cpp
 ../apitemview.cpp
 ../apitemmodel.cpp
 ../scanwidget.cpp
 ../apitemdelegate.cpp
 ../profileitemmodel.cpp
 ../networkprofile.cpp
 ../manageprofilewidget.cpp
 ../encryptionsettingswidget.cpp
 ../wirelesssettingswidget.cpp
 ../generalsettingswidget.cpp
 ../editprofilewidget.cpp
)

kde4_add_ui_files(testManageProfiles_SRCS ../scanwidget.ui)
kde4_add_executable(testManageProfiles ${testManageProfiles_SRCS})

target_link_libraries(testManageProfiles  ${KDE4_KDEUI_LIBS} solidcontrol)
