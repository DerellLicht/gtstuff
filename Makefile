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

CSRC=gtstuff.cpp config.cpp gfuncs.cpp alg_selector.cpp gobjects.cpp palettes.cpp \
circles.cpp squares.cpp

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
palettes.o: palettes.h
circles.o: der_libs/common.h palettes.h gobjects.h gfuncs.h alg_selector.h
squares.o: der_libs/common.h palettes.h gobjects.h gfuncs.h alg_selector.h
der_libs/common_funcs.o: der_libs/common.h
der_libs/common_win.o: der_libs/common.h der_libs/commonw.h
der_libs/statbar.o: der_libs/common.h der_libs/commonw.h der_libs/statbar.h
