#!/bin/sh
$EXTRACTRC `find ui -name '*.ui'` >> rc.cpp
$XGETTEXT `find ui -name '*.cpp'` -o $podir/libknmui.pot
rm -f rc.cpp
