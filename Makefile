## LE MAKEFILE ULTIME ##

# EDIT : Ce Makefile est confirmé de fonctionner sous Linux et Windows.
#
# Normalement il n'y a plus besoin d'y toucher :
# Créez juste vos fichiers dans src/imagimp et laissez-lui le sale boulot ;)
#
# Aussi, il n'y a pas lieu de changer la variable d'environnement
# LD_LIBRARY_PATH (ce que je trouve super intrusif). 
# J'utilise l'option '-rpath' de 'ld' à la place, que je passe grace à 
# l'option '-Wl' de GCC. Ca marche tout aussi bien.


SYS = $(shell gcc -dumpmachine)
ifneq (,$(findstring mingw,$(SYS)))
OS=windows
else
OS=unix
endif

ifeq ($(OS),windows)
	MKDIRP=mkdir
	SLASH=\\
	EXE_EXTENSION=.exe
	LIBDIR=bin
	DLL_PREFIX=
	DLL_EXTENSION=.dll
	LIBGL=opengl32
	LIBGLU=glu32
	LIBGLUT=freeglut
	FPIC=
	CLEANCMD = if exist obj ( rmdir /Q /S obj $(LIBDIR) )
else
	MKDIRP=mkdir -p
	SLASH=/
	EXE_EXTENSION=
	LIBDIR=lib
	DLL_PREFIX=lib
	DLL_EXTENSION=.so
	LIBGL=GL
	LIBGLU=GLU
	LIBGLUT=glut
	FPIC=-fPIC
	CLEANCMD = rm -rf obj $(LIBDIR)
endif

.PHONY: all clean mrproper re dirs

CC = gcc
CFLAGS = -g -std=gnu99 -Wall -Isrc
LDLIBS = -L$(LIBDIR) -Wl,-rpath,$(LIBDIR) -Wl,-rpath,../$(LIBDIR) -l$(LIBGLUT) -lglimagimp -lm
EXE = bin/imagimp$(EXE_EXTENSION)
LIBGLIMAGIMP = $(LIBDIR)/$(DLL_PREFIX)glimagimp$(DLL_EXTENSION)


all: $(LIBGLIMAGIMP) $(EXE)


dirs: bin lib obj$(SLASH)glimagimp obj$(SLASH)imagimp obj$(SLASH)GL
obj:
	$(MKDIRP) $@
obj$(SLASH)glimagimp: | obj
	$(MKDIRP) $@
obj$(SLASH)imagimp: | obj
	$(MKDIRP) $@
obj$(SLASH)GL: | obj
	$(MKDIRP) $@
lib:
	$(MKDIRP) $@
bin:
	$(MKDIRP) $@


obj/imagimp/%.o : src/imagimp/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(patsubst src/%.c,obj/%.o,$(wildcard src/imagimp/*.c))
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

obj/GL/%.o: src/GL/%.c | dirs
	$(CC) $(CFLAGS) -DGLEW_BUILD $(FPIC) -c $< -o $@
obj/glimagimp/%.o: src/glimagimp/%.c | dirs
	$(CC) $(CFLAGS) -DGLEW_BUILD $(FPIC) -c $< -o $@

$(LIBGLIMAGIMP): obj/glimagimp/interface.o \
			     obj/glimagimp/outils.o \
				 obj/GL/glew.o
	$(CC) $(CFLAGS) $(FPIC) -shared $^ -o $@ -l$(LIBGL) -l$(LIBGLU) -l$(LIBGLUT) -lm

clean:
	$(CLEANCMD)
mrproper: clean all
re: mrproper
