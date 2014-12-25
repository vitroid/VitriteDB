#!/bin/sh

#graphquenchを何度か呼びだし、座標の最良推定を求める。

BINDIR=/usr/local/lib/vitrite
file=$1
temp=/tmp/graphquench$$
best=0
energy=100000
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29
do
    $BINDIR/graphquench --nocoord < $file 2> /dev/null | cat $file - | $BINDIR/graphquench2 2> /dev/null | $BINDIR/graphquench4c 1> $temp.$i.tmp 2> $temp.err
    result=`tail -1 $temp.err | grep system`
    if [ "$result" = "" ]
    then
    newenergy=`tail -1 $temp.err | awk '{print int($1*10000)}'`
    echo $i $newenergy
    if [ $newenergy -lt $energy ]
    then
	energy=$newenergy
	best=$i
    fi
    fi
done
#echo "#average residual distortion"
#echo $energy
cat $temp.$best.tmp
rm $temp.*.tmp $temp.err
