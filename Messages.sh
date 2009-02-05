#!/bin/sh
$EXTRACTRC `find libs applet -name '*.ui'` >> rc.cpp
$XGETTEXT `find libs applet -name "*.cpp"` -o $podir/plasma_applet_networkmanagement.pot
rm -f rc.cpp
