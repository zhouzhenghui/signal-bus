#!/bin/sh
set -x
aclocal -I ./m4
autoheader
autoconf
automake --foreign --add-missing --copy
