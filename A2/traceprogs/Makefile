SRCS = simpleloop.c matmul.c blocked.c avlinsertion.c
PROGS = simpleloop matmul blocked avlinsertion

all : $(PROGS)

$(PROGS) : % : %.c
	gcc -Wall -g -o $@ $<


traces: $(PROGS)
	./runit simpleloop
	./runit matmul 100
	./runit blocked 100 25
	./runit avlinsertion

.PHONY: clean
clean : 
	rm -f simpleloop matmul blocked tr-*.ref *.marker *~
