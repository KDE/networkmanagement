#! /usr/bin/env bash
$EXTRACTRC `find . -name '*.ui' -o -name '*.rc'` >> rc.cpp
$XGETTEXT *.cpp -o $podir/kcm_knetworkmanager.po
rm -f rc.cpp
