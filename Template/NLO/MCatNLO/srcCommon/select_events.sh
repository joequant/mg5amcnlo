#!/bin/sh

g77 -o select_events select_events.f mcatnlo_str.f handling_lhe_events.f
./select_events

ofile="select_events.out"
nevents=0

exec <$ofile
read name
read type
read nevents

if [ $type == 1 ];then
 char=".S"
elif [ $type == 2 ];then
 char=".H"
elif [ $type == 3 ];then
 char=".RED"
else
 echo "Wrong itype"
fi

evtfile=$name$char
mv $evtfile $evtfile.old
sed "s/abcdeABCDE/  $nevents/g" $evtfile.old > $evtfile
rm -f $evtfile.old
rm -f $ofile
rm -f select_events