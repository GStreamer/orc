#!/bin/sh

if [ `whoami` = ds ] ; then
  confargs="$confargs --enable-gtk-doc"
fi

autoreconf -i -f &&
./configure --disable-static $confargs $@

