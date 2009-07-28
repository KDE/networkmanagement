#!/bin/sh
$XGETTEXT *.cpp deprecated/*.cpp -o $podir/plasma_applet_networkmanagement.pot
rm -f rc.cpp
