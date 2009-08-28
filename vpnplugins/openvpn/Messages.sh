#!/bin/sh
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp
$XGETTEXT *.cpp -o $podir/networkmanagement_openvpnui.pot
rm -f rc.cpp
