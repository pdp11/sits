CPPFLAGS=-I../tool/libword
LIBWORD=../tool/libword/libword.a
CFLAGS=-g

all: convert-disk analyze-disk punch

convert-disk: convert-disk.o $(LIBWORD)

analyze-disk: analyze-disk.o $(LIBWORD)

punch: punch.o $(LIBWORD)
