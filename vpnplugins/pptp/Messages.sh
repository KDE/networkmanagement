#!/bin/sh
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp
$XGETTEXT *.cpp -o $podir/networkmanagement_pptpui.pot
rm -f rc.cpp
