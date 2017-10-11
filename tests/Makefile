CFLAGS=-g -DDEBUG

all: cmdline ip-cidr-parser regex_ipandcidr

cmdline: cmdline.o
	$(CC) -g -o $@ $^

ip-cidr-parser: ip-cidr-parser.o
	$(CC) -g -o $@ $^

regex_ipandcidr: regex_ipandcidr.o
	$(CC) -O2 -o $@ $^

ip-cidr-parser.o: ip-cidr-parser.c
cmdline.o: cmdline.c

regex_ipandcidr.o: regex_ipandcidr.c cycle_counting.h
	$(CC) -O2 -fno-stack-protector -c -o $@ $<

.PHONY: clean dist-clean

clean:
	-rm *.o

distclean: clean
	rm ip-cidr-parser cmdline regex_ipandcidr
