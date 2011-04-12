# library knmsettingsservice providing access to the dbus interface of settings services
# Used by the Applet to access UserSettings and SystemSettings and provide a list of connections for the user to activate
# And by the Config to access SystemSettings so the user can import+modify system defined settings
#add_subdirectory(settingsservice)

# library knmstorage providing Connection load and save functionality, used by the Config (save/load) and the Service (load)
add_subdirectory(internals)

# library knmui containing widgets for editing connection settings groups, used by the Config
add_subdirectory(ui)

# library knmservice containing components for providing a connection service
add_subdirectory(service)

# library knmclient providing remote interfaces to Activatables provided by the service
add_subdirectory(client)

# Icons used by the tray icons and the Plasmoid
add_subdirectory(icons)
