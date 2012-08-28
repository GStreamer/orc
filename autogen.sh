#!/bin/sh

if [ `whoami` = ds ] ; then
  confargs="$confargs --enable-gtk-doc"
fi

# install pre-commit hook for doing clean commits
if test ! \( -x .git/hooks/pre-commit -a -L .git/hooks/pre-commit \);
then
    rm -f .git/hooks/pre-commit
    ln -s ../../misc/pre-commit.hook .git/hooks/pre-commit
fi

autoreconf -i -f &&
./configure --disable-static $confargs $@

