PROGRAM = main

all:
	lcc -S -D__KLIBC__ -I/usr/local/share/lcc1802/include -I/usr/local/share/klibc/include -I/usr/local/share/klibc/klibc $(PROGRAM).c
	asl -q -cpu 1802 -i /usr/local/share/lcc1802/include -L $(PROGRAM).s
	p2hex -q $(PROGRAM).p
	./mkadr.pl $(PROGRAM).lst > $(PROGRAM).adr

clean:
	rm -f $(PROGRAM) *.{adr,s,p,lst,hex}
