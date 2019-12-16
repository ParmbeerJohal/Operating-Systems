.phony all:
all: mts

mts: mts.c
	gcc -Wall -pthread mts.c -o mts

.PHONY clean:
clean:
	-rm -rf *.o *.exe
