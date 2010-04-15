#!/bin/sh

autoreconf -i -f &&
./configure --disable-static --enable-gtk-doc $@

