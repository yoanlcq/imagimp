#include "Imagimp.h"
#include "glimagimp/interface.h"

Imagimp imagimp;

void sur_clavier(unsigned char ascii, int x, int y) {
    imagimp.fonction_clavier(&imagimp, ascii, x, y);
}
void sur_clavier_special(int touche, int x, int y) {
    imagimp.fonction_clavier_special(&imagimp, touche, x, y);
}
void sur_souris(int bouton, int appuye, int x, int y) {
    imagimp.fonction_souris(&imagimp, bouton, appuye, x, y);
}
void sur_dessin(void) {
    imagimp.fonction_dessin(&imagimp);
}

int main(int argc, char *argv[]) {
    fixeFonctionClavier(sur_clavier);
    fixeFonctionClavierSpecial(sur_clavier_special);
    fixeFonctionClicSouris(sur_souris);
    fixeFonctionDessin(sur_dessin);
    return Imagimp_lancer(&imagimp, argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}
