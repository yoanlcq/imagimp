#ifndef IMAGIMP_H
#define IMAGIMP_H

typedef struct Imagimp Imagimp;
struct Imagimp {
    void (*fonction_clavier)(Imagimp *imagimp, unsigned char ascii, int x, int y);
    void (*fonction_clavier_special)(Imagimp *imagimp, int touche, int x, int y);
    void (*fonction_souris)(Imagimp *imagimp, int bouton, int appuye, int x, int y);
    void (*fonction_dessin)(Imagimp *imagimp);
};

void Imagimp_lancer(Imagimp *imagimp, int argc, char *argv[]);
void Imagimp_fonctionClavierTexte(Imagimp *imagimp, unsigned char ascii, int x, int y);
void Imagimp_fonctionClavier(Imagimp *imagimp, unsigned char ascii, int x, int y);
void Imagimp_fonctionClavierSpecial(Imagimp *imagimp, int touche, int x, int y);
void Imagimp_fonctionSouris(Imagimp *imagimp, int bouton, int appuye, int x, int y);
void Imagimp_fonctionDessin(Imagimp *imagimp);


#endif /* IMAGIMP_H */
