# OpenBOR v4.0 Build 7555 (DCurrent/openbor b1676f97, 2024-01)
E7555 := $(CORE_DIR)/engines/7555

ENGINE_7555_NAME  := v4.0 Build 7555
ENGINE_7555_FLAGS := -fcommon \
	-I$(E7555)/libretro \
	-I$(E7555) \
	-I$(E7555)/source \
	-I$(E7555)/source/adpcmlib \
	-I$(E7555)/source/gamelib \
	-I$(E7555)/source/preprocessorlib \
	-I$(E7555)/source/ramlib \
	-I$(E7555)/source/randlib \
	-I$(E7555)/source/scriptlib \
	-I$(E7555)/source/openborscript \
	-I$(E7555)/source/pnglib \
	-I$(E7555)/source/webmlib

ENGINE_7555_SOURCES := \
	$(E7555)/openbor.c \
	$(E7555)/openborscript.c \
	$(E7555)/source/adpcmlib/adpcm.c \
	$(E7555)/source/gamelib/anigif.c \
	$(E7555)/source/gamelib/bitmap.c \
	$(E7555)/source/gamelib/draw.c \
	$(E7555)/source/gamelib/draw16.c \
	$(E7555)/source/gamelib/draw32.c \
	$(E7555)/source/gamelib/filecache.c \
	$(E7555)/source/gamelib/font.c \
	$(E7555)/source/gamelib/loadimg.c \
	$(E7555)/source/gamelib/models.c \
	$(E7555)/source/gamelib/packfile.c \
	$(E7555)/source/gamelib/palette.c \
	$(E7555)/source/gamelib/pixelformat.c \
	$(E7555)/source/gamelib/screen.c \
	$(E7555)/source/gamelib/screen16.c \
	$(E7555)/source/gamelib/screen32.c \
	$(E7555)/source/gamelib/soundmix.c \
	$(E7555)/source/gamelib/sprite.c \
	$(E7555)/source/gamelib/spriteq.c \
	$(E7555)/source/gamelib/spritex8p16.c \
	$(E7555)/source/gamelib/spritex8p32.c \
	$(E7555)/source/gamelib/transform.c \
	$(E7555)/source/gamelib/translation.c \
	$(E7555)/source/omath.c \
	$(E7555)/source/openborscript/animation.c \
	$(E7555)/source/openborscript/axis.c \
	$(E7555)/source/openborscript/binding.c \
	$(E7555)/source/openborscript/collision_attack.c \
	$(E7555)/source/openborscript/collision_body.c \
	$(E7555)/source/openborscript/collision_entity.c \
	$(E7555)/source/openborscript/colorset.c \
	$(E7555)/source/openborscript/commands.c \
	$(E7555)/source/openborscript/common_property.c \
	$(E7555)/source/openborscript/constants.c \
	$(E7555)/source/openborscript/drawmethod.c \
	$(E7555)/source/openborscript/entity.c \
	$(E7555)/source/openborscript/faction.c \
	$(E7555)/source/openborscript/global_config.c \
	$(E7555)/source/openborscript/icon.c \
	$(E7555)/source/openborscript/index.c \
	$(E7555)/source/openborscript/layer.c \
	$(E7555)/source/openborscript/level.c \
	$(E7555)/source/openborscript/math.c \
	$(E7555)/source/openborscript/model.c \
	$(E7555)/source/openborscript/recursive_damage.c \
	$(E7555)/source/openborscript/spawn_hud.c \
	$(E7555)/source/openborscript/status_dial.c \
	$(E7555)/source/openborscript/string.c \
	$(E7555)/source/pnglib/pngdec.c \
	$(E7555)/source/pnglib/savepng.c \
	$(E7555)/source/preprocessorlib/pp_expr.c \
	$(E7555)/source/preprocessorlib/pp_lexer.c \
	$(E7555)/source/preprocessorlib/pp_parser.c \
	$(E7555)/source/ramlib/ram.c \
	$(E7555)/source/randlib/rand32.c \
	$(E7555)/source/scriptlib/ImportCache.c \
	$(E7555)/source/scriptlib/Instruction.c \
	$(E7555)/source/scriptlib/Interpreter.c \
	$(E7555)/source/scriptlib/Lexer.c \
	$(E7555)/source/scriptlib/List.c \
	$(E7555)/source/scriptlib/Parser.c \
	$(E7555)/source/scriptlib/ParserSet.c \
	$(E7555)/source/scriptlib/ScriptVariant.c \
	$(E7555)/source/scriptlib/Stack.c \
	$(E7555)/source/scriptlib/StackedSymbolTable.c \
	$(E7555)/source/scriptlib/SymbolTable.c \
	$(E7555)/source/stringptr.c \
	$(E7555)/source/stristr.c \
	$(E7555)/source/utils.c \
	$(E7555)/libretro/glue.c \
	$(E7555)/libretro/video.c \
	$(E7555)/libretro/control.c \
	$(E7555)/libretro/sblaster.c \
	$(E7555)/libretro/timer.c
