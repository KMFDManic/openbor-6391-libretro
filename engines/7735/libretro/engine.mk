# OpenBOR v4.0 Build 7735 (DCurrent/openbor f64d63c2, 2025-05-04)
E7735 := $(CORE_DIR)/engines/7735

ENGINE_7735_NAME  := v4.0 Build 7735
ENGINE_7735_FLAGS := -fcommon \
	-I$(E7735)/libretro \
	-I$(E7735) \
	-I$(E7735)/source \
	-I$(E7735)/source/adpcmlib \
	-I$(E7735)/source/gamelib \
	-I$(E7735)/source/preprocessorlib \
	-I$(E7735)/source/ramlib \
	-I$(E7735)/source/randlib \
	-I$(E7735)/source/scriptlib \
	-I$(E7735)/source/openborscript \
	-I$(E7735)/source/pnglib \
	-I$(E7735)/source/webmlib

ENGINE_7735_SOURCES := \
	$(E7735)/openbor.c \
	$(E7735)/openborscript.c \
	$(E7735)/source/adpcmlib/adpcm.c \
	$(E7735)/source/gamelib/anigif.c \
	$(E7735)/source/gamelib/bitmap.c \
	$(E7735)/source/gamelib/draw.c \
	$(E7735)/source/gamelib/draw16.c \
	$(E7735)/source/gamelib/draw32.c \
	$(E7735)/source/gamelib/filecache.c \
	$(E7735)/source/gamelib/font.c \
	$(E7735)/source/gamelib/loadimg.c \
	$(E7735)/source/gamelib/models.c \
	$(E7735)/source/gamelib/packfile.c \
	$(E7735)/source/gamelib/palette.c \
	$(E7735)/source/gamelib/pixelformat.c \
	$(E7735)/source/gamelib/screen.c \
	$(E7735)/source/gamelib/screen16.c \
	$(E7735)/source/gamelib/screen32.c \
	$(E7735)/source/gamelib/soundmix.c \
	$(E7735)/source/gamelib/sprite.c \
	$(E7735)/source/gamelib/spriteq.c \
	$(E7735)/source/gamelib/spritex8p16.c \
	$(E7735)/source/gamelib/spritex8p32.c \
	$(E7735)/source/gamelib/transform.c \
	$(E7735)/source/gamelib/translation.c \
	$(E7735)/source/omath.c \
	$(E7735)/source/openborscript/animation.c \
	$(E7735)/source/openborscript/audio.c \
	$(E7735)/source/openborscript/axis.c \
	$(E7735)/source/openborscript/binding.c \
	$(E7735)/source/openborscript/collision_attack.c \
	$(E7735)/source/openborscript/collision_body.c \
	$(E7735)/source/openborscript/collision_coordinates.c \
	$(E7735)/source/openborscript/collision_entity.c \
	$(E7735)/source/openborscript/colorset.c \
	$(E7735)/source/openborscript/commands.c \
	$(E7735)/source/openborscript/common_property.c \
	$(E7735)/source/openborscript/constants.c \
	$(E7735)/source/openborscript/drawmethod.c \
	$(E7735)/source/openborscript/entity.c \
	$(E7735)/source/openborscript/faction.c \
	$(E7735)/source/openborscript/flash.c \
	$(E7735)/source/openborscript/global_config.c \
	$(E7735)/source/openborscript/icon.c \
	$(E7735)/source/openborscript/index.c \
	$(E7735)/source/openborscript/layer.c \
	$(E7735)/source/openborscript/level.c \
	$(E7735)/source/openborscript/math.c \
	$(E7735)/source/openborscript/model.c \
	$(E7735)/source/openborscript/recursive_damage.c \
	$(E7735)/source/openborscript/spawn_hud.c \
	$(E7735)/source/openborscript/status_dial.c \
	$(E7735)/source/openborscript/string.c \
	$(E7735)/source/pnglib/pngdec.c \
	$(E7735)/source/pnglib/savepng.c \
	$(E7735)/source/preprocessorlib/pp_expr.c \
	$(E7735)/source/preprocessorlib/pp_lexer.c \
	$(E7735)/source/preprocessorlib/pp_parser.c \
	$(E7735)/source/ramlib/ram.c \
	$(E7735)/source/randlib/rand32.c \
	$(E7735)/source/scriptlib/ImportCache.c \
	$(E7735)/source/scriptlib/Instruction.c \
	$(E7735)/source/scriptlib/Interpreter.c \
	$(E7735)/source/scriptlib/Lexer.c \
	$(E7735)/source/scriptlib/List.c \
	$(E7735)/source/scriptlib/Parser.c \
	$(E7735)/source/scriptlib/ParserSet.c \
	$(E7735)/source/scriptlib/ScriptVariant.c \
	$(E7735)/source/scriptlib/Stack.c \
	$(E7735)/source/scriptlib/StackedSymbolTable.c \
	$(E7735)/source/scriptlib/SymbolTable.c \
	$(E7735)/source/stringptr.c \
	$(E7735)/source/stristr.c \
	$(E7735)/source/utils.c \
	$(E7735)/libretro/glue.c \
	$(E7735)/libretro/video.c \
	$(E7735)/libretro/control.c \
	$(E7735)/libretro/sblaster.c \
	$(E7735)/libretro/timer.c
