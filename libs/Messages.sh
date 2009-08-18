#!/bin/sh
$EXTRACTRC `find ui -name '*.ui'` >> rc.cpp
$EXTRACTRC `find internals -name '*.kcfg'` >> rc.cpp
$XGETTEXT `find ui -name '*.cpp'` rc.cpp -o $podir/libknmui.pot
rm -f rc.cpp
