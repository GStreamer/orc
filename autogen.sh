#!/bin/sh

set -e

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

autoreconf -i -f

cd "$olddir"

if test -z "$NOCONFIGURE"; then
    exec $srcdir/configure --disable-static --enable-maintainer-mode --enable-gtk-doc $@
fi

