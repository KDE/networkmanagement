The structure of the networkmanager applet is is (roughly):

networkmanager: paint icon, initialize the extenderitem inside it
nmextenderitem: holds interfaceitems and the tab-thing for activatables
activatablelist: scrollwidget holding the list of activatables, used as tabs
activatableitem: base class for painting activatable
(wireless)interfaceitem: paint an interface, in the left column
wirelessnetworkitem: specialised activatableitem for wifi

Happy hacking!