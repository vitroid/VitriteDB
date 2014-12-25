#!/bin/sh

BINDIR=/usr/local/lib/vitrite
#read @RNGS instead of @NGPH
$BINDIR/rngs2ar3a.pl | $BINDIR/graphquench2 | $BINDIR/graphquench4c 
