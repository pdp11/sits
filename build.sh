#!/bin/sh

../its/tools/dasm/palx -A < files/sits/sits.bin > sits.abs
../its/tools/dasm/palx -A < files/rug/ar.bin > rug.abs
../its/tools/dasm/palx -A < files/sits/salv.bin > salv.abs

../its/tools/simh/BIN/pdp11 salv.simh
../its/tools/simh/BIN/pdp11 rug.simh
