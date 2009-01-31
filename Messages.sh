#! /usr/bin/env bash
$EXTRACTRC `find . -name '*.ui' -o -name '*.rc'` >> rc.cpp
$XGETTEXT *.cpp `find libs -name '*.cpp'` applet/*.cpp -o $podir/plasma_applet_networkmanager.pot
rm -f rc.cpp
