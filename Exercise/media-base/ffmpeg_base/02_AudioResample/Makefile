

FILE 	= $(notdir $(shell ls ./))
SRCFILE	= $(filter %.c,$(FILE))

OFILE   = $(patsubst %.c,%.o,%(SRCFILE))
LIBFILE = $(patsubst %.c,lib%.a,$(SRCFILE))
TARGET  = $(shell pwd | xargs basename)

FFMPEGLIB_PATH	= /usr/local/ffmpeg/lib/
FFMPEGLIB		= $(patsubst %.a,%,$(notdir $(shell ls $(FFMPEGLIB_PATH)/*.a)))

CC		= gcc
#centos7:
#CFLAG 	= -std=c99 -g -lasound -lm -ldl -lstdc++ -lpthread -lxcb-shm -lxcb-xfixes -lxcb-shape -lxcb -lva -lva-x11 -lva-drm -lXext -lfdk-aac -lX11 -lXv -lx264#-lxcb-randr
#CFLAG	+= 'pkg-config --cflags --libs $(FFMPEGLIB)'

#ubuntu20(可能是gcc限制，-l动态库必须放置在被引用库的后面...):
#CFLAG 	= -g -lasound -lm -ldl -lstdc++ -lpthread -lxcb -lXext -lfdk-aac -lX11 -lXv -lx264 -lz #-lxcb-randr
#20220618
#--发现ffmpeg相关lib对应的pkgconfig.pc文件中有列出依赖的动态库,是否有方案可以自动将其转化为CFLAG?
#pkg-config --libs --static libavdevice  --递归输出自身依赖的所以其他包信息,有较多重复项,需要进行优化
#-L/usr/local/ffmpeg/lib -L/usr/local/lib -L/usr/local/ffmpeg/lib -L/usr/local/lib -L/usr/local//lib -L/usr/local/lib -L/usr/local/ffmpeg/lib \
#-lavdevice -lm -lxcb -lasound -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lXv -lX11 -lXext -lavfilter -pthread -lm -lswscale -lm -lpostproc -lm -lavformat -lm -lz -lavcodec -pthread -lm -lz -lfdk-aac -lspeex -lx264 -lx265 -lswresample -lm -lavutil -pthread -lm -lXv -lX11 -lXext

#20220619
#之前的编译命令有问题，使用了静态链接ffmpeglib的方式，静态链接的方式需要链接时找到所有符号，因此ffmpeglib
#本身依赖的动态库无法找到，进一步导致各种符号找不到的编译问题
#修改为动态链接ffmpeglib的方式，则只需要将直接依赖的ffmpeglib链接进来即可，后继依赖在程序加载过程有ld自动
#动态完成(当然需要首先将环境变量LD_LIBARARY_PATH配置ok)
CFLAG	+= -g -ldl
CFLAG	+= $(shell pkg-config $(FFMPEGLIB) --cflags --libs)

INCFILE += -I../include

#REFLIB	= $(FFMPEGLIB)
#REFLIB	+= $(shell ls /usr/lib/*.a)
#REFLIB	+= $(shell ls /usr/local/lib/*.a)

#$(OFILE):$(SRC)

$(TARGET):
	@echo ------------------------------------------------
	@echo target:$(TARGET)
	@echo ------------------------------------------------
#	$(CC) -o $@ $(SRCFILE) $(INCFILE) -Wl,--start-group $(FFMPEGLIB) -Wl,--end-group $(REFLIB)  $(CFLAG)
	$(CC) -o $@ $(SRCFILE) $(INCFILE) $(CFLAG)
all:$(TARGET)

clean:
	rm -rf $(filter-out $(SRCFILE) Makefile, $(FILE))

.PHONY: all

