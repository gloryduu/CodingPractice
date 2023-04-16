

FILE 	= $(notdir $(shell ls ./))
SRCFILE	= $(filter %.c,$(FILE))

OFILE   = $(patsubst %.c,%.o,%(SRCFILE))
LIBFILE = $(patsubst %.c,lib%.a,$(SRCFILE))
TARGET  = $(shell pwd | xargs basename)

CC		= gcc
CFLAG 	= -std=c99

INCFILE = -I/usr/local/ffmpeg/include
INCFILE += -I../include

REFLIB	= $(shell ls /usr/local/ffmpeg/lib/*.a)

#$(OFILE):$(SRC)

$(TARGET):
	@echo ------------------------------------------------
	@echo target:$(TARGET)
	@echo ------------------------------------------------
	$(CC) $(CFLAG) -o $@ $(SRCFILE) $(INCFILE) $(REFLIB)

all:$(TARGET)

clean:
	rm -rf $(filter-out $(SRCFILE) Makefile, $(FILE))

.PHONY: all
