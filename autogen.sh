#!/bin/sh

autoreconf -i -f &&
./configure --disable-static $confargs $@

