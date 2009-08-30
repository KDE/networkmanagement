#!/bin/sh
$EXTRACTRC `find ui -name '*.ui'` >> rc.cpp
$EXTRACTRC `find internals -name '*.kcfg'` >> rc.cpp
$XGETTEXT `find ui -name '*.cpp'` rc.cpp -o $podir/libknmui.pot
$XGETTEXT `find service -name '*.cpp'` rc.cpp -o $podir/libknmservice.pot
$XGETTEXT `find internals -name '*.cpp'` rc.cpp -o $podir/libknminternals.pot
rm -f rc.cpp
