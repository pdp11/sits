#!/bin/sh

PIDS=""

terminal() {
    xterm -e "sleep $3; ./term.sh $1 '$2'" &
    PIDS="$PIDS $!"
    trap "kill $PIDS" EXIT QUIT INT TERM
}

terminal 20002 "DH11 teletype 0" 0
terminal 20002 "DH11 teletype 1" 2
terminal 20002 "DH11 teletype 2" 3

./simh/BIN/pdp11 sits.simh
#gdb ./simh/BIN/pdp11
