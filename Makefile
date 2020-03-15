########################################################################
####################### Makefile Template ##############################
########################################################################

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# Compiler settings - Can be customized.
CC = g++
CXXFLAGS = -Wfatal-errors -Wno-narrowing -Wno-write-strings -Wno-literal-suffix \
	-fpermissive -D__linux__ -m32 -D_JK2MP \
	-DQAGAME -DLUGORMOD -DLMD_NEW_WEAPONS -DLMD_NEW_FORCEPOWERS \
	-DLMD_NEW_SKILLSYS -D_JK2 -DJK2AWARDS -D_GAME

CXXFLAGS_DEBUG = -g -D_DEBUG -Wfatal-errors -Wno-narrowing -Wno-write-strings -Wno-literal-suffix \
	-fpermissive -D__linux__ -m32 -D_JK2MP \
	-DQAGAME -DLUGORMOD -DLMD_NEW_WEAPONS -DLMD_NEW_FORCEPOWERS \
	-DLMD_NEW_SKILLSYS -D_JK2 -DJK2AWARDS -D_GAME

LDFLAGS = -shared -lm -pthread -L$(ROOT_DIR)/lua -llua

# Makefile settings - Can be customized.
APPNAME = jampgamei386.so
EXT = .cpp
SRCDIR = $(ROOT_DIR)
OBJDIR = obj

############## Do not change anything from here downwards! #############
SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
DEP = $(OBJ:$(OBJDIR)/%.o=%.d)
# UNIX-based OS variables & settings
RM = rm
DELOBJ = $(OBJ)
# Windows OS variables & settings
DEL = del
EXE = .exe
WDELOBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)\\%.o)

########################################################################
####################### Targets beginning here #########################
########################################################################

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

################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	$(RM) $(DELOBJ) $(DEP) $(APPNAME)

# Cleans only all files with the extension .d
.PHONY: cleandep
cleandep:
	$(RM) $(DEP)

#################### Cleaning rules for Windows OS #####################
# Cleans complete project
.PHONY: cleanw
cleanw:
	$(DEL) $(WDELOBJ) $(DEP) $(APPNAME)$(EXE)

# Cleans only all files with the extension .d
.PHONY: cleandepw
cleandepw:
	$(DEL) $(DEP)