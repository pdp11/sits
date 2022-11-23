#!/bin/sh

test -r simh/makefile || git submodule update --init --recursive

(cd tool; make)
(cd tools; make)
(cd simh; make pdp11 tt2500)

mkdir -p media

./tool/palx -A < bin/ar.bin > media/rug.abs
./tool/palx -A < bin/salv.bin > media/salv.abs
./tool/palx -A < bin/sits.bin > media/sits.abs

> do/ptr.do

punch() {
    F="$1"
    P="media/$F.pt"
    set `./tools/punch bin/$1.bin 2>&1 > $P`
    echo noexpect >> do/ptr.do
    echo 'expect "*" at ptr '$P'; send "\\033LP:\\r"; continue' >> do/ptr.do
    echo 'expect "*" send "\\033Y0:'$F'\\r"' >> do/ptr.do
    yes continue | head -$2 >> do/ptr.do
}

punch sysspr
punch sits
punch salv
punch ddt
punch fnt
punch slogo
punch jotto
punch dired
punch inquir
punch tinte
punch teco
punch hlogo

./simh/BIN/pdp11 do/install.simh
