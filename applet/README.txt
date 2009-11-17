The structure of the networkmanager applet is is (roughly):

networkmanager: paint icon, initialize the extenderitem inside it
nmextenderitem: holds interfaceitems and the tab-thing for activatables
activatablelist: scrollwidget holding the list of activatables, used as tabs
activatableitem: base class for painting activatable
(wireless)interfaceitem: paint an interface, in the left column
wirelessnetworkitem: specialised activatableitem for wifi

Happy hacking!



= TODO =

* kill right icon in connectionitem
* fix updating of state of connectionitems
* indicate default route in interfaceitem
* fix layouting / scrollwidget for Connections and Wireless Networks tab
* regression: disconnect button doesn't work anymore (stopped working after the /sidsconnect/deactivate patch)
* integrate nuno/ademmer's new artwork
* use network-wired-active for active wired connections
* integrate hiddenwirelessitem into wireless networks tab

== somewhat lower priority ==
* signals for networking enabled changed seems missing
* unavailable for wired -> unplugged

