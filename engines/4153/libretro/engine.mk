# OpenBOR v3.0 Build 4153 (DCurrent/openbor 4a276c8b, 2015-04-23)
E4153 := $(CORE_DIR)/engines/4153

ENGINE_4153_NAME  := v3.0 Build 4153
ENGINE_4153_FLAGS := -fcommon -std=gnu89 \
	-I$(E4153)/libretro \
	-I$(E4153) \
	-I$(E4153)/source \
	-I$(E4153)/source/adpcmlib \
	-I$(E4153)/source/gamelib \
	-I$(E4153)/source/gfxlib \
	-I$(E4153)/source/preprocessorlib \
	-I$(E4153)/source/ramlib \
	-I$(E4153)/source/randlib \
	-I$(E4153)/source/scriptlib \
	-I$(E4153)/source/pnglib \
	-I$(E4153)/source/webmlib

ENGINE_4153_SOURCES := \
	$(E4153)/openbor.c \
	$(E4153)/openborscript.c \
	$(E4153)/source/adpcmlib/adpcm.c \
	$(E4153)/source/gamelib/draw.c \
	$(E4153)/source/gamelib/draw16.c \
	$(E4153)/source/gamelib/draw32.c \
	$(E4153)/source/gamelib/font.c \
	$(E4153)/source/gamelib/translation.c \
	$(E4153)/source/gamelib/anigif.c \
	$(E4153)/source/gamelib/bitmap.c \
	$(E4153)/source/gamelib/screen.c \
	$(E4153)/source/gamelib/screen16.c \
	$(E4153)/source/gamelib/screen32.c \
	$(E4153)/source/gamelib/loadimg.c \
	$(E4153)/source/gamelib/palette.c \
	$(E4153)/source/gamelib/packfile.c \
	$(E4153)/source/gamelib/filecache.c \
	$(E4153)/source/gamelib/pixelformat.c \
	$(E4153)/source/gamelib/soundmix.c \
	$(E4153)/source/gamelib/spriteq.c \
	$(E4153)/source/gamelib/sprite.c \
	$(E4153)/source/gamelib/spritex8p16.c \
	$(E4153)/source/gamelib/spritex8p32.c \
	$(E4153)/source/gamelib/commands.c \
	$(E4153)/source/gamelib/models.c \
	$(E4153)/source/gamelib/transform.c \
	$(E4153)/source/scriptlib/StackedSymbolTable.c \
	$(E4153)/source/scriptlib/ScriptVariant.c \
	$(E4153)/source/scriptlib/SymbolTable.c \
	$(E4153)/source/scriptlib/Instruction.c \
	$(E4153)/source/scriptlib/Interpreter.c \
	$(E4153)/source/scriptlib/ImportCache.c \
	$(E4153)/source/scriptlib/ParserSet.c \
	$(E4153)/source/scriptlib/Parser.c \
	$(E4153)/source/scriptlib/Lexer.c \
	$(E4153)/source/scriptlib/Stack.c \
	$(E4153)/source/scriptlib/List.c \
	$(E4153)/source/preprocessorlib/pp_lexer.c \
	$(E4153)/source/preprocessorlib/pp_parser.c \
	$(E4153)/source/preprocessorlib/pp_expr.c \
	$(E4153)/source/ramlib/ram.c \
	$(E4153)/source/randlib/rand32.c \
	$(E4153)/source/pnglib/pngdec.c \
	$(E4153)/source/pnglib/savepng.c \
	$(E4153)/source/stringptr.c \
	$(E4153)/source/utils.c \
	$(E4153)/source/stristr.c \
	$(E4153)/libretro/glue.c \
	$(E4153)/libretro/video.c \
	$(E4153)/libretro/control.c \
	$(E4153)/libretro/sblaster.c \
	$(E4153)/libretro/timer.c
