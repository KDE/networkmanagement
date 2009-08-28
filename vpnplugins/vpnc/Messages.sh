#!/bin/sh
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp
$XGETTEXT *.cpp -o $podir/networkmanagement_vpncui.pot
rm -f rc.cpp
