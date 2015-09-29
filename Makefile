DEBUG = TRUE
CC = gcc
AS = as
CXX= g++
LD = ld
# CFLAGS = -Wall -W -fPIC --std=c99 `sdl-config --cflags`
# LDFLAGS = -Wall -W -shared --std=c99 `sdl-config --libs`
CFLAGS = -Wall -W --std=c99 `sdl-config --cflags`
LDFLAGS = -Wall -W --std=c99 `sdl-config --libs`
ifeq ($(DEBUG),FALSE)
	CFLAGS += -Os
else
	CFLAGS += -O0 -g -D DEBUG
#    LDFLAGS += --debug
endif
CSOURCES = $(wildcard *.c) $(wildcard */*.c)
ASMSOURCES = $(wildcard *.S) $(wildcard */*.S)
CPPOBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.cpp,%.o,$(wildcard */*.cpp))
OBJS = $(patsubst %.c,%.o,$(CSOURCES)) $(patsubst %.S,%.o,$(ASMSOURCES)) $(CPPOBJS)

ifneq ($(strip $(CPPOBJS)),)
	LDFLAGS += --cpp
endif
# EXE = gem.so
# DISTDIR = lib
EXE = chip
DISTDIR = bin
vpath %.tns $(DISTDIR) 

all: $(EXE)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(AS) -c $< -o $@

$(EXE): $(OBJS)
	mkdir -p $(DISTDIR)
# 	$(LD) $^ -o $(DISTDIR)/$@ $(LDFLAGS)
# ifeq ($(DEBUG),FALSE)
# 	@rm -f $(DISTDIR)/*.gdb
# endif
	$(CC) $^ -o $(DISTDIR)/$@ $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJS) *.elf $(DISTDIR)/*.gdb $(DISTDIR)/$(EXE)
