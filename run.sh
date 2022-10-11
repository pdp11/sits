#!/bin/sh

xterm -e "./term.sh 20001" &
PID1="$!"
xterm -e "./term.sh 20002" &
PID2="$!"
trap "kill $PID1 $PID2" EXIT QUIT INT TERM

./simh/BIN/pdp11 sits.simh
#gdb ./simh/BIN/pdp11
