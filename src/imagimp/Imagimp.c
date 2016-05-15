#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <GL/glut.h>
#include "glimagimp/interface.h"
#include "glimagimp/outils.h"
#include "Imagimp.h"
#include "ImageRVB.h"

static void Imagimp_usage(const char *nom_prog) {
    fprintf(stderr, "Usage: %s <img.ppm>\n"
                    "Sinon: %s :<largeur>x<hauteur>\n", 
                    nom_prog, nom_prog);
}

bool Imagimp_lancer(Imagimp *imagimp, int argc, char *argv[]) {
    if(argc <= 1) {
        Imagimp_usage(argv[0]);
        return false;
    }
    unsigned l, h;
    ImageRVB chargee;
    if(argv[1][0]==':') {
        if(sscanf(argv[1]+1, "%ux%u", &l, &h) < 2) {
            Imagimp_usage(argv[0]);
            return false;
        }
        ImageRVB_allouer(&chargee, l, h);
        ImageRVB_remplirRVB(&chargee, 255, 255, 255);
    } else {
        if(!ImageRVB_importerPPM(&chargee, argv[1]))
            return false;
        l=chargee.l, h=chargee.h;
    }
    imagimp->fonction_clavier = Imagimp_fonctionClavier;
    imagimp->fonction_clavier_special = Imagimp_fonctionClavierSpecial;
    imagimp->fonction_souris = Imagimp_fonctionSouris;
    imagimp->fonction_dessin = Imagimp_fonctionDessin;
    imagimp->largeur_ihm = 300;
    PileCalques_allouer(&imagimp->calques, l, h);
    ImageRVB_remplirEchiquier(&imagimp->calques.virtuel, 16, 64, 150);
    ImageRVB_desallouer(&imagimp->calques.courant->img_source);
    imagimp->calques.courant->img_source.rvb = chargee.rvb;
    Calque_recalculer(imagimp->calques.courant);
    PileCalques_recalculer(&imagimp->calques);
#define img (imagimp->calques.rendu_gl)
    initGLIMAGIMP_IHM(img.l, img.h, img.rvb, img.l+imagimp->largeur_ihm, img.h+20);
#undef img
    return true;
}

void Imagimp_fonctionClavierTexte(Imagimp *imagimp, unsigned char ascii, int x, int y) {
    /* Rien pour l'instant. */
}
/* On peut pas prendre un const Imagimp parce que actualiseImage() ne prend
 * pas un tableau const. Shame. */
static void Imagimp_actualiserAffichageCanevas(Imagimp *imagimp) {
    actualiseImage(imagimp->calques.rendu_gl.rvb);
}
void Imagimp_fonctionClavier(Imagimp *imagimp, unsigned char ascii, int x, int y) {
    printf("Touche : '%c' (souris: %d, %d)\n", ascii, x, y);
    switch(ascii) {
    case 'i': 
        /* (IM_1) Charger une image dans le calque courant. */
        ImageRVB_desallouer(&imagimp->calques.courant->img_source);
        ImageRVB_importerPPM(&imagimp->calques.courant->img_source,
                             "images/Phoenix.512.ppm");
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'v': /* (IHM_1) Vue source-calque ou image-finale */ 
        /* On passe d'une mode de vue à l'autre, parmi :
         * - Voir l'image source du calque courant;
         * - Voir l'image finale; */
        break;
    case 'q': /* (IHM_4) Quitter, en libérant la mémoire. */ 
        PileCalques_desallouerTout(&imagimp->calques);
        exit(EXIT_SUCCESS);
        break;
    case 'c': /* (CAL_1) Ajouter un calque vierge */ 
        /* Calque multiplicatif blanc d'opacité 0, inséré à la fin de la
         * liste de calques. */
        PileCalques_ajouterCalqueVierge(&imagimp->calques);
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'x': /* (CAL_5) Supprimer le calque actif */ 
        /* On ne peut supprimer le dernier calque. */
        PileCalques_supprimerCalqueCourant(&imagimp->calques);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'm': /* (CAL_4) Changer la fonction de mélange du calque */ 
        /* J'aurais bien voulu faire un switch mais on peut pas. */
        if(imagimp->calques.courant->melange == Melange_normal)
            imagimp->calques.courant->melange = Melange_addition;
        else if(imagimp->calques.courant->melange == Melange_addition)
            imagimp->calques.courant->melange = Melange_produit;
        else if(imagimp->calques.courant->melange == Melange_produit)
            imagimp->calques.courant->melange = Melange_normal;
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'l': /* (LUT_1) Ajouter une LUT au calque actif */ 
        /* ADDLUM, DIMLUM, ADDCON, DIMCON, INVERT, SEPIA */
        ListeLUTs_ajouterDerniere(&imagimp->calques.courant->luts);
        imagimp->calques.courant->luts.derniere->fonction = LUT_inversion;
        imagimp->calques.courant->luts.derniere->param1 = 127;
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 't': 
        if(!imagimp->calques.courant->luts.derniere)
            break;
#define TROP_LONG (imagimp->calques.courant->luts.derniere->fonction)
             if(TROP_LONG == LUT_inversion)
                TROP_LONG  = LUT_augmentationLuminosite;
        else if(TROP_LONG == LUT_augmentationLuminosite)
                TROP_LONG  = LUT_diminutionLuminosite;
        else if(TROP_LONG == LUT_diminutionLuminosite)
                TROP_LONG  = LUT_augmentationContraste;
        else if(TROP_LONG == LUT_augmentationContraste)
                TROP_LONG  = LUT_diminutionContraste;
        else if(TROP_LONG == LUT_diminutionContraste)
                TROP_LONG  = LUT_sepia;
        else if(TROP_LONG == LUT_sepia)
                TROP_LONG  = LUT_inversion;
#undef TROP_LONG
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'o': 
        if(imagimp->calques.courant->luts.derniere->param1>0)
            --(imagimp->calques.courant->luts.derniere->param1);
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'p': 
        if(imagimp->calques.courant->luts.derniere->param1<255)
            ++(imagimp->calques.courant->luts.derniere->param1);
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'L': /* (LUT_3) Supprimer la dernière LUT au calque actif */ 
        ListeLUTs_retirerDerniere(&imagimp->calques.courant->luts);
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'a': /* (LUT_2) Appliquer une LUT au calque actif */ 
        Calque_appliquerPremiereLUT(imagimp->calques.courant);
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
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
        imagimp->calques.courant->opacite -= .02f;
        if(imagimp->calques.courant->opacite < 0.f)
            imagimp->calques.courant->opacite = 0.f;
        printf("Opacité : %u%%\n", (unsigned)(imagimp->calques.courant->opacite*100.f));
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case GLUT_KEY_RIGHT:
        /* (CAL_3) Opacité d'un calque */
        imagimp->calques.courant->opacite += .02f;
        if(imagimp->calques.courant->opacite > 1.f)
            imagimp->calques.courant->opacite = 1.f;
        printf("Opacité : %u%%\n", (unsigned)(imagimp->calques.courant->opacite*100.f));
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
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

void Imagimp_fonctionDessin(Imagimp *imagimp) {
    fixeCouleur(1, 1, 1);
    writeString(.5f, .975f, "Foo !!");
#define img (imagimp->calques.rendu)
#define ihm_l (imagimp->largeur_ihm)
    float histo_debut = 1.f-ihm_l/(float)(img.l+ihm_l);
    float histo_fin = 1.f;
    float histo_pas = (histo_fin-histo_debut)/256.f;
    float histo_hauteur = 64.f;
    fixeCouleur(.8f, .8f, .8f);
    drawCarre(histo_debut, 1.f-histo_hauteur/img.h, 
              histo_fin, 1.f);
    Histogramme histo;
    ImageRVB_histogrammeRVB(&imagimp->calques.rendu, histo);
    uint32_t histo_max;
    size_t i;
    for(histo_max=i=0 ; i<256 ; ++i)
        if(histo_max < histo[i])
            histo_max = histo[i];
    fixeCouleur(.2f, .2f, .2f);
    for(i=0 ; i<256 ; ++i)
        drawLigne(histo_debut+i*histo_pas, 1.f-histo_hauteur/img.h,
                  histo_debut+i*histo_pas, 1.f-(histo_hauteur-histo[i]*histo_hauteur/histo_max)/img.h);
#undef img
#undef ihm_l
}
