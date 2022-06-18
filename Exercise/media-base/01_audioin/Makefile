

FILE 	= $(notdir $(shell ls ./))
SRCFILE	= $(filter %.c,$(FILE))

OFILE   = $(patsubst %.c,%.o,%(SRCFILE))
LIBFILE = $(patsubst %.c,lib%.a,$(SRCFILE))
TARGET  = $(shell pwd | xargs basename)

#FFMPEGLIB	= $(notdir $(shell ls /usr/local/ffmpeg/lib/*.a))
FFMPEGLIB	= $(shell ls /usr/local/ffmpeg/lib/*.a)

CC		= gcc
#centos7:
#CFLAG 	= -std=c99 -g -lasound -lm -ldl -lstdc++ -lpthread -lxcb-shm -lxcb-xfixes -lxcb-shape -lxcb -lva -lva-x11 -lva-drm -lXext -lfdk-aac -lX11 -lXv -lx264#-lxcb-randr
#CFLAG	+= 'pkg-config --cflags --libs $(FFMPEGLIB)'
#CFLAG	+= 'pkg-config --cflags --libs libavcodec libavdevice libavfilter libavformat libavutil libpostproc libswresample libswscale'

#ubuntu20(可能是gcc限制，-l动态库必须放置在被引用库的后面...):
CFLAG 	= -g -lasound -lm -ldl -lstdc++ -lpthread -lxcb -lXext -lfdk-aac -lX11 -lXv -lx264 -lz #-lxcb-randr

INCFILE = -I/usr/local/ffmpeg/include
INCFILE += -I../include

#REFLIB	= $(FFMPEGLIB)
#REFLIB	+= $(shell ls /usr/lib/*.a)
REFLIB	+= $(shell ls /usr/local/lib/*.a)

#$(OFILE):$(SRC)

$(TARGET):
	@echo ------------------------------------------------
	@echo target:$(TARGET)
	@echo ------------------------------------------------
	$(CC) -o $@ $(SRCFILE) $(INCFILE) -Wl,--start-group $(FFMPEGLIB) -Wl,--end-group $(REFLIB)  $(CFLAG)
all:$(TARGET)

clean:
	rm -rf $(filter-out $(SRCFILE) Makefile, $(FILE))

.PHONY: all

