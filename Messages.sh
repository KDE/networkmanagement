#! /usr/bin/env bash
$EXTRACTRC `find . -name '*.ui' -o -name '*.rc'` >> rc.cpp
$XGETTEXT *.cpp libs/ui/*.cpp libs/ui/security/*.cpp libs/storage/*.cpp applet/*.cpp settings/service/*.cpp settings/config/*.cpp -o $podir/plasma_applet_networkmanager.pot
rm -f rc.cpp
