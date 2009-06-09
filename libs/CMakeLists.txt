# library knmsettingsservice providing access to the dbus interface of settings services
# Used by the Applet to access UserSettings and SystemSettings and provide a list of connections for the user to activate
# And by the Config to access SystemSettings so the user can import+modify system defined settings
#add_subdirectory(settingsservice)

# library knmstorage providing Connection load and save functionality, used by the Config (save/load) and the Service (load)
add_subdirectory(internals)

# library knmui containing widgets for editing connection settings groups, used by the Config
add_subdirectory(ui)

# library knmdbus containing interface classes to dbus objects specified by NetworkManager
add_subdirectory(dbus)

