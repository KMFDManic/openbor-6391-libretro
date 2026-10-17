# OpenBOR v3.0 Build 3711 (DCurrent/openbor 6ec17fa4, 2012-07-04)
E3711 := $(CORE_DIR)/engines/3711

ENGINE_3711_NAME  := v3.0 Build 3711
ENGINE_3711_FLAGS := -fcommon -std=gnu89 \
	-I$(E3711)/libretro \
	-I$(E3711) \
	-I$(E3711)/source \
	-I$(E3711)/source/adpcmlib \
	-I$(E3711)/source/gamelib \
	-I$(E3711)/source/gfxlib \
	-I$(E3711)/source/preprocessorlib \
	-I$(E3711)/source/ramlib \
	-I$(E3711)/source/randlib \
	-I$(E3711)/source/scriptlib \
	-I$(E3711)/source/pnglib

ENGINE_3711_SOURCES := \
	$(E3711)/openbor.c \
	$(E3711)/openborscript.c \
	$(E3711)/source/adpcmlib/adpcm.c \
	$(E3711)/source/gamelib/draw.c \
	$(E3711)/source/gamelib/draw16.c \
	$(E3711)/source/gamelib/draw32.c \
	$(E3711)/source/gamelib/font.c \
	$(E3711)/source/gamelib/anigif.c \
	$(E3711)/source/gamelib/bitmap.c \
	$(E3711)/source/gamelib/screen.c \
	$(E3711)/source/gamelib/screen16.c \
	$(E3711)/source/gamelib/screen32.c \
	$(E3711)/source/gamelib/loadimg.c \
	$(E3711)/source/gamelib/palette.c \
	$(E3711)/source/gamelib/packfile.c \
	$(E3711)/source/gamelib/filecache.c \
	$(E3711)/source/gamelib/pixelformat.c \
	$(E3711)/source/gamelib/soundmix.c \
	$(E3711)/source/gamelib/spriteq.c \
	$(E3711)/source/gamelib/sprite.c \
	$(E3711)/source/gamelib/spritex8p16.c \
	$(E3711)/source/gamelib/spritex8p32.c \
	$(E3711)/source/gamelib/commands.c \
	$(E3711)/source/gamelib/models.c \
	$(E3711)/source/gamelib/transform.c \
	$(E3711)/source/gamelib/triangle.c \
	$(E3711)/source/scriptlib/StackedSymbolTable.c \
	$(E3711)/source/scriptlib/ScriptVariant.c \
	$(E3711)/source/scriptlib/SymbolTable.c \
	$(E3711)/source/scriptlib/Instruction.c \
	$(E3711)/source/scriptlib/Interpreter.c \
	$(E3711)/source/scriptlib/ImportCache.c \
	$(E3711)/source/scriptlib/ParserSet.c \
	$(E3711)/source/scriptlib/Parser.c \
	$(E3711)/source/scriptlib/Lexer.c \
	$(E3711)/source/scriptlib/Stack.c \
	$(E3711)/source/scriptlib/List.c \
	$(E3711)/source/preprocessorlib/pp_lexer.c \
	$(E3711)/source/preprocessorlib/pp_parser.c \
	$(E3711)/source/ramlib/ram.c \
	$(E3711)/source/randlib/rand32.c \
	$(E3711)/source/pnglib/pngdec.c \
	$(E3711)/source/pnglib/savepng.c \
	$(E3711)/source/stringptr.c \
	$(E3711)/source/utils.c \
	$(E3711)/source/stristr.c \
	$(E3711)/libretro/glue.c \
	$(E3711)/libretro/video.c \
	$(E3711)/libretro/control.c \
	$(E3711)/libretro/sblaster.c \
	$(E3711)/libretro/timer.c
