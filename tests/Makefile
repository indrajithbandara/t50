CFLAGS=-O2 -fno-stack-protector -mtune=native

all: cmdline ip-cidr-parser regex_ipandcidr

cmdline: cmdline.o
	$(CC) -o $@ $^

ip-cidr-parser: ip-cidr-parser.o
	$(CC) -o $@ $^

regex_ipandcidr: regex_ipandcidr.o
	$(CC) -o $@ $^

ip-cidr-parser.o: ip-cidr-parser.c cycle_counting.h
cmdline.o: cmdline.c
regex_ipandcidr.o: regex_ipandcidr.c cycle_counting.h

.PHONY: clean dist-clean

clean:
	-rm *.o

distclean: clean
	rm ip-cidr-parser cmdline regex_ipandcidr
