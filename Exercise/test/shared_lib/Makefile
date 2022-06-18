

all:so a stabin

a:atest
stabin:statest
so :sotest

FILE = $(notdir $(shell ls ./))
SRCFILE  = $(filter %.c,$(FILE))

ACCFILE = data.c
SOCCFILE= sum.c
OFILE   = $(patsubst %.c,%.o,%(SRCFILE))
SOFILE  = $(patsubst %.c,lib%.so,$(SOCCFILE))
LIBFILE = $(patsubst %.c,lib%.a,$(ACCFILE))
CFLAG 	= -std=c99

#$(OFILE):$(SRCFILE)

$(SOFILE):$(SOCCFILE)
	gcc $(CFLAG) -shared -fPIC -o $@ $<
	gcc $(CFLAG) -shared -o $@_nopic $<

$(LIBFILE):$(ACCFILE)
	gcc $(CFLAG) -c $^
	ar -rcs $@ *.o
	#rm *.o

atest:$(LIBFILE) $(SOFILE)
	gcc $(CFLAG) -o $@ main_static.c $(LIBFILE) $(SOFILE)

statest:
	gcc $(CFLAG) -static -o $@ main_static.c sum.c

sotest:$(LIBFILE) $(SOFILE)
	gcc $(CFLAG) -o $@ main.c $(LIBFILE) -ldl

clean:
	rm -rf $(filter-out $(SRCFILE) Makefile, $(FILE))

.PHONY: all so a stabin clean
