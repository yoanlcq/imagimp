#include <stdio.h>
#include <stdint.h>
#include <GL/glut.h>
#include "glimagimp/interface.h"
#include "Imagimp.h"
#include "ImageRVB.h"

void Imagimp_lancer(Imagimp *imagimp, int argc, char *argv[]) {
    imagimp->fonction_clavier = Imagimp_fonctionClavier;
    imagimp->fonction_clavier_special = Imagimp_fonctionClavierSpecial;
    imagimp->fonction_souris = Imagimp_fonctionSouris;
    imagimp->fonction_dessin = Imagimp_fonctionDessin;
    const size_t l=512, h=512, ihm_l=200;
    PileCalques_allouer(&imagimp->calques, l, h);
#define img (imagimp->calques.rendu)
    initGLIMAGIMP_IHM(img.l, img.h, img.rvb, img.l+ihm_l, img.h);
#undef img
}

void Imagimp_fonctionClavierTexte(Imagimp *imagimp, unsigned char ascii, int x, int y) {
    /* Rien pour l'instant. */
}
void Imagimp_fonctionClavier(Imagimp *imagimp, unsigned char ascii, int x, int y) {
    printf("Touche : '%c' (souris: %d, %d)\n", ascii, x, y);
    switch(ascii) {
    case 'i': 
        /* (IM_1) Charger une image dans le calque courant. */
        ImageRVB_desallouer(&imagimp->calques.courant->img_source);
        ImageRVB_importerPPM(&imagimp->calques.courant->img_source,
                             "images/SailBoat.512.ppm");
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        break;
    case 'v': /* (IHM_1) Vue source-calque ou image-finale */ break;
    case 'q': /* (IHM_4) Quitter, en libérant la mémoire. */ 
        PileCalques_desallouerTout(&imagimp->calques);
        exit(EXIT_SUCCESS);
        break;
    case 'c': /* (CAL_1) Ajouter un calque vierge */ 
        /* Calque multiplicatif blanc d'opacité 0, inséré à la fin de la
         * liste de calques. */
        PileCalques_ajouterCalqueVierge(&imagimp->calques);
        break;
    case 'x': /* (CAL_5) Supprimer le calque actif */ 
        /* On ne peut supprimer le dernier calque. */
        PileCalques_supprimerCalqueCourant(&imagimp->calques);
        break;
    case 'm': /* (CAL_4) Changer la fonction de mélange du calque */ 
        /* J'aurais bien voulu faire un switch mais on peut pas. */
        if(imagimp->calques.courant->melange == Melange_normal)
            imagimp->calques.courant->melange = Melange_addition;
        else if(imagimp->calques.courant->melange == Melange_addition)
            imagimp->calques.courant->melange = Melange_produit;
        else if(imagimp->calques.courant->melange == Melange_produit)
            imagimp->calques.courant->melange = Melange_normal;
        break;
    case 'l': /* (LUT_1) Ajouter une LUT au calque actif */ 
        /* ADDLUM, DIMLUM, ADDCON, DIMCON, INVERT, SEPIA */
        ListeLUTs_ajouterDerniere(&imagimp->calques.courant->luts);
        imagimp->calques.courant->luts.derniere->fonction = LUT_inversion;
        imagimp->calques.courant->luts.derniere->param1 = 127;
        break;
    /* Changer les paramètres de la dernière LUT */
    case 't': 
#define TROP_LONG (imagimp->calques.courant->luts.derniere->fonction)
        if(TROP_LONG == LUT_inversion)
            TROP_LONG = LUT_augmentationLuminosite;
        else if(TROP_LONG == LUT_augmentationLuminosite)
            TROP_LONG = LUT_diminutionLuminosite;
        else if(TROP_LONG == LUT_diminutionLuminosite)
            TROP_LONG = LUT_augmentationContraste;
        else if(TROP_LONG == LUT_augmentationContraste)
            TROP_LONG = LUT_diminutionContraste;
        else if(TROP_LONG == LUT_diminutionContraste)
            TROP_LONG = LUT_sepia;
        else if(TROP_LONG == LUT_sepia)
            TROP_LONG = LUT_inversion;
#undef TROP_LONG
        break;
    case 'o': 
        if(imagimp->calques.courant->luts.derniere->param1>0)
            --(imagimp->calques.courant->luts.derniere->param1);
        break;
    case 'p': 
        if(imagimp->calques.courant->luts.derniere->param1<255)
            ++(imagimp->calques.courant->luts.derniere->param1);
        break;
    case 'L': /* (LUT_3) Supprimer la dernière LUT au calque actif */ 
        ListeLUTs_retirerDerniere(&imagimp->calques.courant->luts);
        break;
    case 'a': /* (LUT_2) Appliquer une LUT au calque actif */ 
        Calque_appliquerPremiereLUT(imagimp->calques.courant);
        break;
    case 's': 
        /* (IM_2) Sauvegarder l'image finale */
        ImageRVB_exporterPPM(&imagimp->calques.rendu, "export.ppm");
        break;

    /* Les features suivantes ne sont plus requises : */
    /* case 'h':*/ /* Imprimer l'historique dans le terminal */ 
        /* L'historique conserve : 
         * IM_1, CAL_1, CAL_3, CAL_4, CAL_5, LUT_1, LUT_3 
         * Action annulables :
         * CAL_1, CAL_3, CAL_4, LUT_1, LUT_3 */
        /* break; */
    /* case 'z':*/ /* Annuler *//* break; */
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
    case GLUT_KEY_LEFT: 
        /* (CAL_3) Opacité d'un calque */ 
        imagimp->calques.courant->opacite -= .05f;
        break;
    case GLUT_KEY_RIGHT:
        /* (CAL_3) Opacité d'un calque */
        imagimp->calques.courant->opacite += .05f;
        break;
    case GLUT_KEY_UP: 
        /* (CAL_2) Calque suivant */
        if(imagimp->calques.courant->au_dessus)
            imagimp->calques.courant = imagimp->calques.courant->au_dessus;
        break;
    case GLUT_KEY_DOWN: 
        /* (CAL_2) Calque précédent */
        if(imagimp->calques.courant->en_dessous)
            imagimp->calques.courant = imagimp->calques.courant->en_dessous;
        break;
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
