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
* use network-wired-active for active wired connections
* integrate hiddenwirelessitem into wireless networks tab
* Check that the plasmoid is robust for
 * NetworkManager restarts
 * dbus restarts
 * kded quit and restart
 * kded module loaded, but unable to obtain NMUserSettings because another client had it
 * network interface kernel module unload/reload
* Condistionally install network.svgz
* update title label when less/more is clicked or details shown

== somewhat lower priority ==
o signals for networking enabled changed seems missing
o unavailable for wired -> unplugged


Happy hacking!

-- sebas
