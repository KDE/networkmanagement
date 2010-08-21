#!/bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/knetworkmanager.pot
rm -f rc.cpp
