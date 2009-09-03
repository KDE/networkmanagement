#!/bin/sh
$EXTRACTRC `find settings -name '*.ui' -o -name '*.rc'` `find vpnplugins -name '*.ui'` >> rc.cpp
$EXTRACTRC `find libs/ui -name '*.ui'` >> rc.cpp
$EXTRACTRC `find libs/internals -name '*.kcfg'` >> rc.cpp
$XGETTEXT `find vpnplugins/ -name '*.cpp'` `find libs -name "*.cpp"` `find settings -name "*.cpp"` rc.cpp -o $podir/libknetworkmanager.pot
