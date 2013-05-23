kde4_install_icons(${ICON_INSTALL_DIR})

if(${KDE_VERSION} VERSION_GREATER 4.10.50)
	add_subdirectory(extra)
endif(${KDE_VERSION} VERSION_GREATER 4.10.50)
