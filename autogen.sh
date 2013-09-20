#!/bin/sh

set -e

if [ `whoami` = ds ] ; then
  confargs="$confargs --enable-gtk-doc"
fi

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

autoreconf -i -f

cd "$olddir"

$srcdir/configure --disable-static $confargs $@

