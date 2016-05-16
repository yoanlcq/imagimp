#ifndef IMAGIMP_H
#define IMAGIMP_H

#include <stdbool.h>
#include "Calque.h"

#define GLIMAGIMP_HAUTEUR_POLICE 12
#define GLIMAGIMP_LARGEUR_POLICE 8

typedef struct Imagimp Imagimp;
#include "Console.h"
struct Imagimp {
    void (*fonction_clavier)(Imagimp *imagimp, unsigned char ascii, int x, int y);
    void (*fonction_clavier_special)(Imagimp *imagimp, int touche, int x, int y);
    void (*fonction_souris)(Imagimp *imagimp, int bouton, int appuye, int x, int y);
    void (*fonction_dessin)(Imagimp *imagimp);
    size_t largeur_ihm, hauteur_lignecmd;
    bool vue_export;
    Console console;
    PileCalques calques;
};

bool Imagimp_lancer(Imagimp *imagimp, int argc, char *argv[]);
void Imagimp_actualiserAffichageCanevas(Imagimp *imagimp);
void Imagimp_fonctionClavierTexte(Imagimp *imagimp, unsigned char ascii, int x, int y);
void Imagimp_fonctionClavierTexteSpecial(Imagimp *imagimp, int touche, int x, int y);
void Imagimp_fonctionClavier(Imagimp *imagimp, unsigned char ascii, int x, int y);
void Imagimp_fonctionClavierSpecial(Imagimp *imagimp, int touche, int x, int y);
void Imagimp_fonctionSouris(Imagimp *imagimp, int bouton, int appuye, int x, int y);
void Imagimp_fonctionDessin(Imagimp *imagimp);


#endif /* IMAGIMP_H */
