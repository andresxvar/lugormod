ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CC = g++
CXXFLAGS = -Wfatal-errors -Wno-narrowing -Wno-write-strings -Wno-literal-suffix \
	-fpermissive -D__linux__ -m32 -D_JK2MP \
	-DQAGAME -DLUGORMOD -DLMD_NEW_WEAPONS -DLMD_NEW_FORCEPOWERS \
	-DLMD_NEW_SKILLSYS -D_JK2 -DJK2AWARDS -D_GAME -g -D_DEBUG 
LDFLAGS = -shared -lm -pthread -L$(ROOT_DIR)/lua -llua

SRCDIR = $(ROOT_DIR)
EXT = .cpp

OBJDIR = obj
OBJEXT = .o

DEPDIR = dep
DEPEXT = .d

APPNAME = build/jampgamei386.so

SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%$(OBJEXT))
DEP = $(OBJ:$(OBJDIR)/%$(OBJEXT)=$(DEPDIR)/%$(DEPEXT))

## command to remove files
# Unix
RM = rm
DELOBJ = $(OBJ)
# Windows
DEL = del
EXE = .exe
WDELOBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)\\%.o)

## targets
all: $(APPNAME)

# Builds the app
$(APPNAME): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Creates the dependecy rules
%.d: $(SRCDIR)/%$(EXT)
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:%.d=$(OBJDIR)/%.o) >$@

# Includes all .h files
-include $(DEP)

# Building rule for .o files and its .c/.cpp in combination with all .h
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT)
	$(CC) $(CXXFLAGS) -o $@ -c $<

## cleanning
.PHONY: clean
clean:
	$(RM) $(DELOBJ)

.PHONY: cleandep
cleandep:
	$(RM) $(DEP)

.PHONY: cleanw
cleanw:
	$(DEL) $(WDELOBJ) $(DEP) $(APPNAME)$(EXE)

.PHONY: cleandepw
cleandepw:
	$(DEL) $(DEP)