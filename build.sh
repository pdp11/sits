#!/bin/sh

SITS=new
RUG=new
#SITS=files/sits
#RUG=files/rug

(cd tool; make)
(cd tools; make)
(cd simh; make pdp11)

./tool/palx -A < $RUG/ar.bin > rug.abs
./tool/palx -A < $SITS/salv.bin > salv.abs
./tool/palx -A < $SITS/sits.bin > sits.abs

./tools/punch $SITS/sysspr.bin > sysspr.pt
./tools/punch $SITS/sits.bin > sits.pt
./tools/punch $SITS/salv.bin > salv.pt
./tools/punch $SITS/ddt.bin > ddt.pt
./tools/punch $SITS/fnt.bin > fnt.pt
./tools/punch $SITS/slogo.bin > slogo.pt
./tools/punch $SITS/jotto.bin > jotto.pt
./tools/punch sits/inquir.bin > inquir.pt

./simh/BIN/pdp11 salv.simh
./simh/BIN/pdp11 rug.simh
