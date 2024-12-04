#!/bin/bash
let MULT=1;
let V=0;
let NPOINTS=$COLUMNS+$COLUMNS;
let NPOINTS=$NPOINTS+1;
rm testdata.txt;
for i in `seq 1 $NPOINTS`; do
	if [ $V -eq 20 ]; then
		let V=19;
		let MULT=-1;
	elif [ $V -eq 1 ]; then
		let V=2;
		let MULT=1;
	else 
		let V=${V}+${MULT};
	fi;
	echo -n "$V  " >> testdata.txt;
done;
echo "" >> testdata.txt
