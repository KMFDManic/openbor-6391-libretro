# OpenBOR (v6391) libretro core
#
#   make                       build for the host (Linux/BSD, macOS, Windows/MinGW)
#   make platform=win          cross/MinGW build
#   make DEBUG=1               unoptimised build with symbols
#   make SYSTEM_ZLIB=1 SYSTEM_LIBPNG=1 SYSTEM_VORBIS=1
#                              link against system libraries instead of deps/

CORE_DIR := .
TARGET_NAME := openbor

ifeq ($(platform),)
   platform = unix
   ifeq ($(shell uname -s),)
      platform = win
   else ifneq ($(findstring MINGW,$(shell uname -s)),)
      platform = win
   else ifneq ($(findstring MSYS,$(shell uname -s)),)
      platform = win
   else ifneq ($(findstring Darwin,$(shell uname -s)),)
      platform = osx
   endif
endif

GIT_VERSION := $(shell git rev-parse --short HEAD 2>/dev/null)
ifneq ($(GIT_VERSION),)
   DEFINES += -DGIT_VERSION=\"\ $(GIT_VERSION)\"
endif

CC ?= gcc
LIBS := -lm

ifeq ($(platform), unix)
   TARGET  := $(TARGET_NAME)_libretro.so
   fpic    := -fPIC
   SHARED  := -shared -Wl,--version-script=$(CORE_DIR)/link.T -Wl,--no-undefined
   DEFINES += -DLINUX
else ifeq ($(platform), osx)
   TARGET  := $(TARGET_NAME)_libretro.dylib
   fpic    := -fPIC
   SHARED  := -dynamiclib
   DEFINES += -DLINUX -DDARWIN
   ifneq ($(arch),)
      CFLAGS  += -arch $(arch)
      LDFLAGS += -arch $(arch)
   endif
else ifeq ($(platform), win)
   TARGET  := $(TARGET_NAME)_libretro.dll
   SHARED  := -shared -static-libgcc -Wl,--version-script=$(CORE_DIR)/link.T -Wl,--no-undefined
   DEFINES += -DWIN
   LIBS    += -lpsapi
else
   $(error Unsupported platform '$(platform)')
endif

include Makefile.common

ifeq ($(SYSTEM_ZLIB), 1)
   LIBS += -lz
endif
ifeq ($(SYSTEM_LIBPNG), 1)
   LIBS += -lpng
endif
ifeq ($(SYSTEM_VORBIS), 1)
   LIBS += -lvorbisfile -lvorbis -logg
endif

DEFINES += -DLIBRETRO -D__LIBRETRO__ -DPNG_NO_CONFIG_H

ifeq ($(DEBUG), 1)
   CFLAGS += -O0 -g -DDEBUG
else
   CFLAGS += -O2 -DNDEBUG
endif

CFLAGS += -std=gnu99 -fsigned-char -fcommon -fno-strict-aliasing -Wall \
          -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function \
          $(fpic) $(DEFINES) $(INCFLAGS)

OBJECTS := $(SOURCES_C:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(fpic) $(SHARED) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
