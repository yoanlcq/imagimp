## LE MAKEFILE ULTIME ##

# Ce Makefile devrait marcher sous Linux, Windaube (MinGW) et OS X.
# Normalement il n'y a plus besoin d'y toucher :
# Créez juste vos fichiers dans src/ et laissez-lui le sale boulot ;)
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
	DLL_PREFIX=
	DLL_EXTENSION=.dll
	LIBGL=opengl32
	CLEANCMD = IF exist obj ( rmdir /Q /S obj lib && $(MKDIRP) obj\glimagimp lib )
else
	MKDIRP=mkdir -p
	SLASH=/
	EXE_EXTENSION=
	DLL_PREFIX=lib
	DLL_EXTENSION=.so
	LIBGL=GL
	CLEANCMD = rm -rf obj/* lib/* && $(MKDIRP) obj/glimagimp
endif

.PHONY: all clean mrproper re dirs

CC = gcc
CFLAGS = -g -Wall -Isrc
LDLIBS = -Llib -Wl,-rpath,lib -lglimagimp -lm
EXE = bin/imagimp$(EXE_EXTENSION)
LIBGLIMAGIMP = lib/$(DLL_PREFIX)glimagimp$(DLL_EXTENSION)


all: $(LIBGLIMAGIMP) $(EXE)


dirs: bin lib obj/glimagimp
obj:
	$(MKDIRP) $@
obj$(SLASH)glimagimp: obj
	$(MKDIRP) $@
lib:
	$(MKDIRP) $@
bin:
	$(MKDIRP) $@


obj/%.o : src/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(addsuffix .o, \
			$(addprefix obj/, \
				$(notdir \
					$(basename \
						$(wildcard src/*.c)))))
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

obj/glimagimp/%.o: src/glimagimp/%.c | dirs
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(LIBGLIMAGIMP): obj/glimagimp/interface.o \
			     obj/glimagimp/outils.o
	$(CC) $(CFLAGS) -fPIC -shared $^ -o $@ -l$(LIBGL) -lGLU -lglut -lm

clean:
	$(CLEANCMD)
mrproper: clean all
re: mrproper
