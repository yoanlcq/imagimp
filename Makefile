# Ce Makefile devrait marcher sous tout OS.
# Sous Windows, il faut que vous utilisiez MinGW.
#
# Il n'y a pas besoin de lui dire à chaque fois quels nouveaux fichiers
# on rajoute.
# Créez juste vos fichiers dans src/ et laissez-lui le sale boulot ;)


SYS = $(shell gcc -dumpmachine)
ifneq (,$(findstring mingw,$(SYS)))
OS=windows
else
OS=unix
endif

ifeq ($(OS),windows)
	EXE_EXTENSION=.exe
	DLL_PREFIX=
	DLL_EXTENSION=.dll
	LIBGL=opengl32
	CLEANCMD = IF exist obj ( rmdir /Q /S obj lib && mkdir obj\glimagimp lib )
else
	EXE_EXTENSION=
	DLL_PREFIX=lib
	DLL_EXTENSION=.so
	LIBGL=GL
	CLEANCMD = rm -rf obj/* lib/* && mkdir obj/glimagimp
endif

.PHONY: all clean mrproper re

CC = gcc
CFLAGS = -g -Wall
LDLIBS = -Llib -Wl,-rpath,lib -lglimagimp -lm
EXE = bin/imagimp$(EXE_EXTENSION)
LIBGLIMAGIMP = lib/$(DLL_PREFIX)glimagimp$(DLL_EXTENSION)


all: $(LIBGLIMAGIMP) $(EXE)

obj/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(addsuffix .o,$(addprefix obj/,$(notdir $(basename $(wildcard src/*.c)))))
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

obj/glimagimp/%.o: src/glimagimp/%.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(LIBGLIMAGIMP): obj/glimagimp/interface.o \
			     obj/glimagimp/outils.o
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -l$(LIBGL) -lGLU -lglut -lm

clean:
	$(CLEANCMD)
mrproper: clean all
re: mrproper
