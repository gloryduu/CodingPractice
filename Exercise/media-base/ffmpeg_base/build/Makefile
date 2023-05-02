
TOP		= ..
BINPATH = $(TOP)/bin

SUBDIRS = $(wildcard $(TOP)/src/*)
TARGETS = $(notdir $(SUBDIRS))

CC		= gcc
CFLAGS 	= -std=c99
CFLAGS 	+= -Wall

#FILE 	= $(notdir $(shell ls $(TOP)/src/$(TGTNAME)))
FILES 	= $(wildcard $(TOP)/src/$(TGTNAME)/*)
CFILES	= $(filter %.c,$(FILES))

OFILES   = $(patsubst %.c,%.o,%(CFILES))
LIBFILES = $(patsubst %.c,lib%.a,$(CFILES))
#TGTNAME = $(shell pwd | xargs basename)
TGTPATH = $(BINPATH)/$(TGTNAME)
TARGET  = $(TGTPATH)/$(TGTNAME)

#INCFILE = -I/usr/local/ffmpeg/include
#REFLIB	= $(shell ls /usr/local/ffmpeg/lib/*.a)
SDKPATH	= /usr/local/ffmpeg/lib/
SDKLIBS = $(patsubst %.a,%,$(notdir $(shell ls $(SDKPATH)/*.a)))

INCPATH = -I$(TOP)/include

CFLAGS	+= $(INCPATH)
CFLAGS	+= $(shell pkg-config $(SDKLIBS) --cflags)
LDFLAGS = $(shell pkg-config $(SDKLIBS) --libs)


#$(OFILE):$(SRC)

$(TARGET):
	@echo ------------------------------------------------
	@echo target:$(TARGET)
	@echo ------------------------------------------------
	@mkdir $(TGTPATH) -p
	$(CC) -o $@ $(CFLAGS) $(CFILES) $(LDFLAGS)

build:$(TARGET)

.PHONY: build