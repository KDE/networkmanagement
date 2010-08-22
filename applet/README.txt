The structure of the networkmanager applet is is (roughly):

networkmanager: paint icon, initialize the extenderitem inside it
nmpopup: holds interfaceitems and the tab-thing for activatables
activatablelistwidget: scrollwidget holding the list of activatables, used as tabs
activatableitem: base class for painting activatable
interfaceconnectionitem: activatableitem for non-wireless connections
wirelessnetworkitem: specialised activatableitem for wifi
(wireless)interfaceitem: paint an interface, in the left column
interfacedetailswidget: show details of the interface, visible when clicked on interface


= TODO =

[ o = done; * = todo ]

o kill right icon in connectionitem
o fix updating of state of connectionitems
o indicate default route in interfaceitem
o fix layouting / scrollwidget for Connections and Wireless Networks tab
o disconnect button disconnects wrong connections
o integrate nuno/ademmer's new artwork
o use SVG artwork for wired connections
* integrate hiddenwirelessitem into wireless networks tab
* Check that the plasmoid is robust for
 * NetworkManager restarts
 * dbus restarts
 * kded quit and restart (seems broken!)
 * kded module loaded, but unable to obtain NMUserSettings because another client had it
 o network interface kernel module unload/reload
o Condistionally install network.svgz
* update title label when less/more is clicked or details shown
* hide show/more less button when it makes sense
* sizehint of dialog: resizable using corner
* check why kded4 module doesn't load automatically
* Find a way to disconnect (instead of automatically reconnect on those auto connections)


== somewhat lower priority ==
o signals for networking enabled changed seems missing
o unavailable for wired -> unplugged


= Wrong SSID bug notes =

The problem is that ConnectionEditor::addConnection picks an existing uuid after kconfigshell is invoked for the first time. Likely causes:
- QUuid::createUuid() produces duplicate uuids, can't be guaranteed unique
- it seems that the first uuid generated is often already there
- I've not seen it happening twice, so in fact just called QUuid::createUuid() before we try to generate connectionId is enough, but better safe than sorry, so we check that now

- a file /home/sebas/.kde4/share/apps/networkmanagement/connections/$NEW_UUID already exists, and triggers updating the config with its value
- the uuid returned by QUuid::createUuid() must not exist there already, otherwise this bug is triggered

== Fix implemented ==
- try harder to find a non-existing uuid for storing the config in ConnectionEditor::addConnection

== Steps to reproduce ==
- tools/show-ap.py, find a valid accesspoint number, which is NOT a current connnection
  ./showap.py 2>&1 |g -v Cannot
- needs to be run TWICE after make to reproduce:
/home/sebas/kdesvn/install/lib/kde4/libexec/networkmanagement_configshell create --type 802-11-wireless --specific-args "/org/freedesktop/NetworkManager/Devices/1 /org/freedesktop/NetworkManager/AccessPoint/9 wifi_pass"                                                                                                                 -
(change AP id from show-ap, or look into tools/invoke-config.py)

== debug output ==
- tail -f ~/.xsession-errors to see the debug output of the configshell
- killall kded4 ; sleep 3; kded4 to see the debug output of what's being invoked (this part should be fine, bug seems to be in configshell or uuid)


== Test ==
- invoke conigshell with valid connection three times
- invoke conigshell with new AP three times
- invoke different new wireless networks from plasmoid

-> verify that always the correct ssid and connection name is set in the UI

don't forget to restart kded4 after rebuilding

Happy hacking!

-- sebas
