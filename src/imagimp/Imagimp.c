#include <stdio.h>
#include <string.h>
#include <GL/glut.h>
#include "glimagimp/interface.h"
#include "Imagimp.h"

void Imagimp_lancer(Imagimp *imagimp, int argc, char *argv[]) {
    imagimp->fonction_clavier = Imagimp_fonctionClavier;
    imagimp->fonction_clavier_special = Imagimp_fonctionClavierSpecial;
    imagimp->fonction_souris = Imagimp_fonctionSouris;
    imagimp->fonction_dessin = Imagimp_fonctionDessin;
    const unsigned w=800, h=600, ihm_w=200;
    unsigned char rvb[3*w*h];
    memset(rvb, 0x8e, 3*w*h);
    initGLIMAGIMP_IHM(w, h, rvb, w+ihm_w, h);
}

void Imagimp_fonctionClavierTexte(Imagimp *imagimp, unsigned char ascii, int x, int y) {}
void Imagimp_fonctionClavier(Imagimp *imagimp, unsigned char ascii, int x, int y) {
    printf("Touche : '%c' (souris: %d, %d)\n", ascii, x, y);
    switch(ascii) {
    case 'l': /* (LUT_1) Ajouter une LUT au calque actif */ 
        /* ADDLUM, DIMLUM, ADDCON, DIMCON, INVERT, SEPIA */
        break;
    case 'L': /* (LUT_3) Supprimer la dernière LUT au calque actif */ break;
    case 'a': /* (LUT_2) Appliquer une LUT au calque actif */ break;
    case 'v': /* (IHM_1) Vue source-calque ou image-finale */ break;
    case 'q': /* (IHM_4) Quitter, en libérant la mémoire. */ break;
    case 'h': /* Imprimer l'historique dans le terminal */ 
        /* L'historique conserve : 
         * IM_1, CAL_1, CAL_3, CAL_4, CAL_5, LUT_1, LUT_3 
         * Action annulables :
         * CAL_1, CAL_3, CAL_4, LUT_1, LUT_3 */
        break;
    case 'z': /* Annuler */ break;
    case 'c': /* (CAL_1) Ajouter un calque vierge */ 
        /* Calque multiplicatif blanc d'opacité 0, inséré à la fin de la
         * liste de calques. */
        break;
    case 'x': /* (CAL_5) Supprimer le calque actif */ 
        /* On ne peut supprimer le dernier calque. */
        break;
    case 'm': /* (CAL_4) Changer la fonction de mélange du calque */ break;
    case 's': /* (IM_2) Sauvegarder l'image finale */ break;
    case 'i': /* (IM_1) Charger une image dans le calque courant. */ break;
    }
}

void Imagimp_fonctionClavierSpecial(Imagimp *imagimp, int touche, int x, int y) {
    printf("Touche spéciale : %d (souris: %d, %d)\n", touche, x, y);
    switch(touche) {
    case GLUT_KEY_F1: break;
    case GLUT_KEY_F2: break;
    case GLUT_KEY_F3: break;
    case GLUT_KEY_F4: break;
    case GLUT_KEY_F5: break; 
    case GLUT_KEY_F6: break;
    case GLUT_KEY_F7: break;
    case GLUT_KEY_F8: break;
    case GLUT_KEY_F9: break;
    case GLUT_KEY_F10: break;
    case GLUT_KEY_F11: break;
    case GLUT_KEY_F12: break;
    case GLUT_KEY_LEFT: /* (CAL_3) Opacité d'un calque */ break;
    case GLUT_KEY_RIGHT: /* (CAL_3) Opacité d'un calque */ break;
    case GLUT_KEY_UP: /* (CAL_2) Calque suivant */ break;
    case GLUT_KEY_DOWN: /* (CAL_2) Calque précédent */ break;
    case GLUT_KEY_PAGE_UP: break;
    case GLUT_KEY_PAGE_DOWN: break;
    case GLUT_KEY_HOME: break;
    case GLUT_KEY_END: break;
    case GLUT_KEY_INSERT: break;
    }
}

void Imagimp_fonctionSouris(Imagimp *imagimp, int bouton, int appuye, int x, int y) {
    printf("clic: %d (%s) vers %d, %d\n", 
           bouton, appuye ? "pressé" : "laché", x, y);
}

void Imagimp_fonctionDessin(Imagimp *imagimp) {}
