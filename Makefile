GCC = g++
CFLAGS = -std=c++11 -Wall -Wextra

all: isaclient isaserver

isaclient:
	$(GCC) $(CFLAGS) isaclient.c chelper.c -o isaclient

isaserver:
	$(GCC) $(CFLAGS) isaserver.c shelper.c -o isaserver

clean:
	rm isaclient isaserver

