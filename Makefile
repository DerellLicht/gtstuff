USE_DEBUG = NO
USE_64BIT = NO
USE_UNICODE = NO
USE_CLANG = YES
# sadly, cygwin mingw does not support gdiplus...
USE_CYGWIN = NO

# the legacy version of qualify.cpp, does not depend upon c++ string class
USE_LEGACY = NO

include der_libs\tool_select.mak

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -g -Weffc++ -c 
LFLAGS=
else
CFLAGS=-Wall -O2 -Weffc++ -c 
LFLAGS=-s -mwindows 
endif
CFLAGS += -Wno-write-strings
#CFLAGS += -Wno-stringop-truncation

ifeq ($(USE_STATIC),YES)
LFLAGS += -static
endif

# link library files
LiFLAGS = -Ider_libs
CFLAGS += -Ider_libs

CSRC=gtstuff.cpp config.cpp gfuncs.cpp alg_selector.cpp gobjects.cpp ezfont.cpp \
palettes.cpp circles.cpp squares.cpp polygon.cpp rect.cpp pixels.cpp colorbars.cpp xpalette.cpp \
rgb_data.cpp bitblt.cpp xnpalette.cpp xrect.cpp gpalettes.cpp triangles.cpp \
rainbow.cpp lines.cpp line_games.cpp rcolors.cpp flames.cpp faces.cpp \
ascii.cpp stained_glass.cpp wincolors.cpp

CSRC+=der_libs/common_funcs.cpp \
der_libs/common_win.cpp \
der_libs/statbar.cpp \
der_libs/winmsgs.cpp 

OBJS = $(CSRC:.cpp=.o) rc.o

BASE=gtstuff
BIN=$(BASE).exe

LIBS=-lgdi32 

#************************************************************
%.o: %.cpp
	$(TOOLS)/$(GNAME) $(CFLAGS) $< -o $@

#************************************************************
all: $(BIN)

clean:
	rm -vf $(BIN) $(OBJS) *.zip *.bak *~

dist:
	rm -f $(BASE).zip
	zip $(BASE).zip *.exe 

wc:
	wc -l *.cpp *.rc

clint:
	cmd /C "python ..\ClaudeLint.py --exclude der_libs"
	
cppc:
	cmd /C "cppcheck --project=compile_commands.json --std=c++14 --suppressions-list=./.suppress.cppcheck"

check:
	cmd /C "d:\llvm\bin\clang-tidy.exe $(CSRC)"

depend:
	makedepend $(CFLAGS) $(CSRC)

#************************************************************
$(BIN): $(OBJS)
	$(TOOLS)/$(GNAME) $(LFLAGS) $(OBJS) -o $@ $(LIBS)

rc.o: $(BASE).rc 
	$(TOOLS)\$(WRNAME) $< -O coff -o $@

# DO NOT DELETE

gtstuff.o: resource.h der_libs/common.h der_libs/commonw.h gtstuff.h gfuncs.h
gtstuff.o: alg_selector.h config.h der_libs/statbar.h der_libs/winmsgs.h
config.o: der_libs/common.h config.h
gfuncs.o: der_libs/common.h gfuncs.h gtstuff.h palettes.h
alg_selector.o: der_libs/common.h resource.h gtstuff.h palettes.h gobjects.h
alg_selector.o: gfuncs.h alg_selector.h
gobjects.o: der_libs/common.h gtstuff.h gfuncs.h palettes.h gobjects.h
ezfont.o: der_libs/common.h gtstuff.h gfuncs.h ezfont.h
palettes.o: palettes.h
circles.o: der_libs/common.h palettes.h gobjects.h gfuncs.h alg_selector.h
squares.o: der_libs/common.h palettes.h gobjects.h gfuncs.h alg_selector.h
polygon.o: der_libs/common.h palettes.h gobjects.h gfuncs.h alg_selector.h
rect.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
rect.o: alg_selector.h
pixels.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
pixels.o: alg_selector.h
colorbars.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
colorbars.o: alg_selector.h
xpalette.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
xpalette.o: alg_selector.h rgb_data.h
rgb_data.o: rgb_data.h
bitblt.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
bitblt.o: alg_selector.h
xnpalette.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
xnpalette.o: alg_selector.h rgb_data.h
xrect.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
xrect.o: alg_selector.h rgb_data.h
gpalettes.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
gpalettes.o: alg_selector.h rgb_data.h cheetah.def hometown.def
triangles.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
triangles.o: alg_selector.h
rainbow.o: der_libs/common.h gtstuff.h gfuncs.h palettes.h gobjects.h
rainbow.o: alg_selector.h ezfont.h
lines.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
lines.o: alg_selector.h
line_games.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
line_games.o: alg_selector.h
rcolors.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
rcolors.o: alg_selector.h
flames.o: der_libs/common.h gtstuff.h gfuncs.h palettes.h gobjects.h
flames.o: alg_selector.h
faces.o: der_libs/common.h gtstuff.h gfuncs.h palettes.h gobjects.h
faces.o: alg_selector.h ezfont.h
ascii.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
ascii.o: alg_selector.h ezfont.h
stained_glass.o: der_libs/common.h gtstuff.h palettes.h gobjects.h gfuncs.h
stained_glass.o: alg_selector.h
wincolors.o: palettes.h gobjects.h gfuncs.h alg_selector.h
der_libs/common_funcs.o: der_libs/common.h
der_libs/common_win.o: der_libs/common.h der_libs/commonw.h
der_libs/statbar.o: der_libs/common.h der_libs/commonw.h der_libs/statbar.h
