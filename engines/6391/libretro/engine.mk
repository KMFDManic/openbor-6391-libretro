# OpenBOR v3.0 Build 6391 (DCurrent/openbor tag v6391, 2018-08-21)
E6391 := $(CORE_DIR)/engines/6391

ENGINE_6391_NAME  := v3.0 Build 6391
ENGINE_6391_FLAGS := -fcommon \
	-I$(E6391)/libretro \
	-I$(E6391) \
	-I$(E6391)/source \
	-I$(E6391)/source/adpcmlib \
	-I$(E6391)/source/gamelib \
	-I$(E6391)/source/preprocessorlib \
	-I$(E6391)/source/ramlib \
	-I$(E6391)/source/randlib \
	-I$(E6391)/source/scriptlib \
	-I$(E6391)/source/openborscript \
	-I$(E6391)/source/pnglib \
	-I$(E6391)/source/webmlib

ENGINE_6391_SOURCES := \
	$(E6391)/openbor.c \
	$(E6391)/openborscript.c \
	$(E6391)/source/adpcmlib/adpcm.c \
	$(E6391)/source/gamelib/draw.c \
	$(E6391)/source/gamelib/draw16.c \
	$(E6391)/source/gamelib/draw32.c \
	$(E6391)/source/gamelib/font.c \
	$(E6391)/source/gamelib/translation.c \
	$(E6391)/source/gamelib/anigif.c \
	$(E6391)/source/gamelib/bitmap.c \
	$(E6391)/source/gamelib/screen.c \
	$(E6391)/source/gamelib/screen16.c \
	$(E6391)/source/gamelib/screen32.c \
	$(E6391)/source/gamelib/loadimg.c \
	$(E6391)/source/gamelib/palette.c \
	$(E6391)/source/gamelib/packfile.c \
	$(E6391)/source/gamelib/filecache.c \
	$(E6391)/source/gamelib/pixelformat.c \
	$(E6391)/source/gamelib/soundmix.c \
	$(E6391)/source/gamelib/spriteq.c \
	$(E6391)/source/gamelib/sprite.c \
	$(E6391)/source/gamelib/spritex8p16.c \
	$(E6391)/source/gamelib/spritex8p32.c \
	$(E6391)/source/gamelib/models.c \
	$(E6391)/source/gamelib/transform.c \
	$(E6391)/source/scriptlib/StackedSymbolTable.c \
	$(E6391)/source/scriptlib/ScriptVariant.c \
	$(E6391)/source/scriptlib/SymbolTable.c \
	$(E6391)/source/scriptlib/Instruction.c \
	$(E6391)/source/scriptlib/Interpreter.c \
	$(E6391)/source/scriptlib/ImportCache.c \
	$(E6391)/source/scriptlib/ParserSet.c \
	$(E6391)/source/scriptlib/Parser.c \
	$(E6391)/source/scriptlib/Lexer.c \
	$(E6391)/source/scriptlib/Stack.c \
	$(E6391)/source/scriptlib/List.c \
	$(E6391)/source/preprocessorlib/pp_lexer.c \
	$(E6391)/source/preprocessorlib/pp_parser.c \
	$(E6391)/source/preprocessorlib/pp_expr.c \
	$(E6391)/source/openborscript/commands.c \
	$(E6391)/source/openborscript/animation.c \
	$(E6391)/source/openborscript/axis.c \
	$(E6391)/source/openborscript/binding.c \
	$(E6391)/source/openborscript/collision_attack.c \
	$(E6391)/source/openborscript/collision_body.c \
	$(E6391)/source/openborscript/collision_entity.c \
	$(E6391)/source/openborscript/constants.c \
	$(E6391)/source/openborscript/energy_status.c \
	$(E6391)/source/openborscript/entity.c \
	$(E6391)/source/openborscript/index.c \
	$(E6391)/source/openborscript/layer.c \
	$(E6391)/source/openborscript/level.c \
	$(E6391)/source/openborscript/math.c \
	$(E6391)/source/openborscript/string.c \
	$(E6391)/source/ramlib/ram.c \
	$(E6391)/source/randlib/rand32.c \
	$(E6391)/source/pnglib/pngdec.c \
	$(E6391)/source/pnglib/savepng.c \
	$(E6391)/source/stringptr.c \
	$(E6391)/source/utils.c \
	$(E6391)/source/stristr.c \
	$(E6391)/source/omath.c \
	$(E6391)/libretro/glue.c \
	$(E6391)/libretro/video.c \
	$(E6391)/libretro/control.c \
	$(E6391)/libretro/sblaster.c \
	$(E6391)/libretro/timer.c
