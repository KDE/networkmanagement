#! /usr/bin/env bash
$EXTRACTRC `find . -name '*.ui' -o -name '*.rc'` >> rc.cpp
$XGETTEXT *.cpp widgets/*.cpp libs/ui/*.cpp libs/storage/*.cpp applet/*.cpp -o $podir/plasma_applet_networkmanager.pot
rm -f rc.cpp
