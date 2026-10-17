# OpenBOR multi-engine libretro core
#
#   make                       build for the host (Linux/BSD, macOS, Windows/MinGW)
#   make ENGINES="6391 7555"   choose the engine versions to include
#   make platform=win          cross/MinGW build
#   make DEBUG=1               unoptimised build with symbols
#   make SYSTEM_ZLIB=1 SYSTEM_LIBPNG=1 SYSTEM_VORBIS=1
#                              link against system libraries instead of deps/

CORE_DIR := .
.DEFAULT_GOAL := all
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
OBJCOPY ?= objcopy
LIBS := -lm
SYM_PREFIX :=

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
   SYM_PREFIX := _
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

DEFINES += -D__LIBRETRO__ -DPNG_NO_CONFIG_H

ifeq ($(DEBUG), 1)
   CFLAGS += -O0 -g -DDEBUG
else
   CFLAGS += -O2 -DNDEBUG
endif

CFLAGS += -std=gnu99 -fsigned-char -fno-strict-aliasing -Wall \
          -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function \
          $(fpic) $(DEFINES) $(INCFLAGS)

OBJECTS := $(SOURCES_C:.c=.o)

# ---------------------------------------------------------------------------
# Each engine version is compiled with its own include paths, partially
# linked into one object and every global symbol except its ob_engine_<ver>
# accessor is made local, so several engines can coexist in the core.
# ---------------------------------------------------------------------------
define ENGINE_RULES
ENGINE_$(1)_OBJECTS := $$(ENGINE_$(1)_SOURCES:.c=.o)
ENGINE_OBJECTS += $(CORE_DIR)/engines/$(1)/ob_engine_$(1).o
CFLAGS += -DHAVE_ENGINE_$(1)

$$(ENGINE_$(1)_OBJECTS): %.o: %.c
	$$(CC) $$(CFLAGS) $$(ENGINE_$(1)_FLAGS) -DLIBRETRO -DOB_ENGINE_ID=$(1) '-DOB_ENGINE_NAME="$$(ENGINE_$(1)_NAME)"' -c -o $$@ $$<

$(CORE_DIR)/engines/$(1)/ob_engine_$(1).o: $$(ENGINE_$(1)_OBJECTS)
ifeq ($(platform), osx)
	ld -r $(LDFLAGS) -exported_symbol $(SYM_PREFIX)ob_engine_$(1) -o $$@ $$^
else
	$$(CC) $(fpic) -nostdlib -r -Wl,-d -o $$@.tmp $$^
	$$(OBJCOPY) --keep-global-symbol=$(SYM_PREFIX)ob_engine_$(1) $$@.tmp $$@
	rm -f $$@.tmp
endif
endef

ENGINE_OBJECTS :=
$(foreach e,$(ENGINES),$(eval $(call ENGINE_RULES,$(e))))

all: $(TARGET)

$(TARGET): $(OBJECTS) $(ENGINE_OBJECTS)
	$(CC) $(fpic) $(SHARED) $(LDFLAGS) -o $@ $(OBJECTS) $(ENGINE_OBJECTS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET) $(ENGINE_OBJECTS) \
	      $(foreach e,$(ENGINES),$(ENGINE_$(e)_OBJECTS))

.PHONY: all clean
