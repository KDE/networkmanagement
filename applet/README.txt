The structure of the networkmanager applet is is (roughly):

networkmanager: paint icon, initialize the extenderitem inside it
nmextenderitem: holds interfaceitems and the tab-thing for activatables
activatablelistwidget: scrollwidget holding the list of activatables, used as tabs
activatableitem: base class for painting activatable
interfaceconnectionitem: activatableitem for non-wireless connections
wirelessnetworkitem: specialised activatableitem for wifi
(wireless)interfaceitem: paint an interface, in the left column



= TODO =

[ o = done; * = todo ]

o kill right icon in connectionitem
o fix updating of state of connectionitems
o indicate default route in interfaceitem
* fix layouting / scrollwidget for Connections and Wireless Networks tab
* disconnect button disconnects wrong connections
* integrate nuno/ademmer's new artwork
* use network-wired-active for active wired connections
* integrate hiddenwirelessitem into wireless networks tab
* Check that the plasmoid is robust for
 * NetworkManager restarts
 * dbus restarts
 * kded quit and restart
 * kded module loaded, but unable to obtain NMUserSettings because another client had it
 * network interface kernel module unload/reload

== somewhat lower priority ==
* signals for networking enabled changed seems missing
* unavailable for wired -> unplugged


Happy hacking!

-- sebas
