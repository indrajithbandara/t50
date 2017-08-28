CFLAGS=-g -DDEBUG

all: cmdline ip-cidr-parser

cmdline: cmdline.o
	$(CC) -g -o $@ $<

ip-cidr-parser: ip-cidr-parser.o
	$(CC) -g -o $@ $<

ip-cidr-parser.o: ip-cidr-parser.c
cmdline.o: cmdline.c

.PHONY: clean dist-clean

clean:
	-rm *.o

distclean: clean
	rm ip-cidr-parser cmdline
