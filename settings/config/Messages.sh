#! /usr/bin/env bash
$EXTRACTRC `find . -name '*.ui' -o -name '*.rc'` >> rc.cpp
$XGETTEXT `find . ../configshell -name "*.cpp"` -o $podir/kcm_networkmanagement.pot
rm -f rc.cpp
