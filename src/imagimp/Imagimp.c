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

    imagimp->largeur_ihm = 256;
    imagimp->hauteur_lignecmd = 14;

    memset(&imagimp->console, 0, sizeof(Console));

    PileCalques_allouer(&imagimp->calques, l, h);
    ImageRVB_remplirEchiquier(&imagimp->calques.virtuel, 16, 64, 150);
    ImageRVB_desallouer(&imagimp->calques.courant->img_source);
    imagimp->calques.courant->img_source.rvb = chargee.rvb;
    Calque_recalculer(imagimp->calques.courant);
    PileCalques_recalculer(&imagimp->calques);
#define img (imagimp->calques.rendu_gl)
    initGLIMAGIMP_IHM(img.l, img.h, img.rvb, img.l+imagimp->largeur_ihm, img.h+2*imagimp->hauteur_lignecmd);
#undef img
    return true;
}

void Imagimp_fonctionClavierTexte(Imagimp *imagimp, unsigned char ascii, int x, int y) {
    switch(ascii) {
    case '\b': Console_enleverCaractere(&imagimp->console); break;
    case '\t': Console_completer(&imagimp->console); break;
    case '\n':
    case '\r':
        Console_executer(&imagimp->console, imagimp);
    case '\033':
        Console_effacerEntree(&imagimp->console);
        imagimp->fonction_clavier = Imagimp_fonctionClavier;
        imagimp->fonction_clavier_special = Imagimp_fonctionClavierSpecial;
        break;
    default:
        Console_insererCaractere(&imagimp->console, ascii);
        break;
    }
}
/* On peut pas prendre un const Imagimp parce que actualiseImage() ne prend
 * pas un tableau const. Shame. */
static void Imagimp_actualiserAffichageCanevas(Imagimp *imagimp) {
    actualiseImage(imagimp->calques.rendu_gl.rvb);
}

/* Attention, cette fonction est gardée pour des raisons d'héritage.
 * Voir Imagimp_fonctionClavierTexte() à la place. */
void Imagimp_fonctionClavier(Imagimp *imagimp, unsigned char ascii, int x, int y) {
    printf("Touche : '%c' (souris: %d, %d)\n", ascii, x, y);
    switch(ascii) {
    case '\t':
        Console_effacerEntree(&imagimp->console);
        imagimp->fonction_clavier = Imagimp_fonctionClavierTexte;
        imagimp->fonction_clavier_special = Imagimp_fonctionClavierTexteSpecial;
        break;
    /*
    case 'i': 
        ImageRVB_desallouer(&imagimp->calques.courant->img_source);
        ImageRVB_importerPPM(&imagimp->calques.courant->img_source,
                             "images/Phoenix.512.ppm");
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'v':
        break;
    case 'q': 
        PileCalques_desallouerTout(&imagimp->calques);
        exit(EXIT_SUCCESS);
        break;
    case 'c':
        PileCalques_ajouterCalqueVierge(&imagimp->calques);
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'x':
        PileCalques_supprimerCalqueCourant(&imagimp->calques);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'm':
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
    case 'l':
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
    case 'L':
        ListeLUTs_retirerDerniere(&imagimp->calques.courant->luts);
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 'a':
        Calque_appliquerPremiereLUT(imagimp->calques.courant);
        Calque_recalculer(imagimp->calques.courant);
        PileCalques_recalculer(&imagimp->calques);
        Imagimp_actualiserAffichageCanevas(imagimp);
        break;
    case 's': 
        ImageRVB_exporterPPM(&imagimp->calques.rendu, "export.ppm");
        break;
        */
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

void Imagimp_fonctionClavierTexteSpecial(Imagimp *imagimp, int touche, int x, int y) {
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
        break;
    case GLUT_KEY_RIGHT:
        break;
    case GLUT_KEY_UP: 
        break;
    case GLUT_KEY_DOWN: 
        break;
    case GLUT_KEY_PAGE_UP: break;
    case GLUT_KEY_PAGE_DOWN: break;
    case GLUT_KEY_HOME: break;
    case GLUT_KEY_END: break;
    case GLUT_KEY_INSERT: break;
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

static void dessinerHistogramme(const Histogramme *histo, float x, float y, float l, float h) {
    const float pas = l/256.f;
    size_t i;
    for(i=0 ; i<256 ; ++i)
        drawLigne(x+i*pas, y-h,
                  x+i*pas, y-h+h*histo->donnees[i]/(float)histo->max);

}
static void dessinerLesHistogrammes(const Imagimp *imagimp, float histo_h) {
    const Histogramme * const histos[4] = {
        &imagimp->calques.histogramme_rendu_rvb,
        &imagimp->calques.histogramme_rendu_r,
        &imagimp->calques.histogramme_rendu_v,
        &imagimp->calques.histogramme_rendu_b,
    };
    const float couleurs[4][3] = {
        {.8f, .8f, .8f},
        {.8f, .0f, .0f},
        {.0f, .8f, .0f},
        {.0f, .0f, .8f}
    };
    size_t i;
    for(i=0 ; i<4 ; ++i) {
        fixeCouleur(couleurs[i][0], couleurs[i][1], couleurs[i][2]);
#define img (imagimp->calques.rendu)
#define ihm_l (imagimp->largeur_ihm)
        dessinerHistogramme(
            histos[i],
            (img.l+1.f)/(float)(img.l+ihm_l), 
            1.f-(imagimp->hauteur_lignecmd*2.f + i*histo_h)/(float)img.h,
            256.f/(img.l+ihm_l),
            histo_h/img.h
        );
#undef img
#undef ihm_l
    }
}
static void dessinerConsole(Console *lc, float h, float img_l, float largeur_ecran, bool activee) {
    float y_offset = h/4.f;
    if(lc->reponse_rouge)
        fixeCouleur(1.f, .2f, .0f);
    else
        fixeCouleur(.0f, .7f, .4f);
    writeString(0.f, 1.f-h+y_offset, lc->reponse);

    fixeCouleur(.9f, .9f, .9f);
    if(!activee) {
        writeString(0.f, 1.f-2.f*h+y_offset, "Entrer une commande avec [TAB].");
        return;
    }
    fixeCouleur(.0f, .0f, .0f);
    drawCarre(0.f, 1.f-(2.f+lc->nb_cmd_suggerees)*h, largeur_ecran, 1.f-2.f*h+y_offset);
    size_t i;
    for(i=0 ; i<lc->nb_cmd_suggerees ; ++i) {
        float x = 0.f; 
        fixeCouleur(0.f, 1.f, 1.f);
        writeString(x, 1.f-(3+i)*h+y_offset, (char*) CONSOLE_CMDS[lc->cmd_suggerees[i]].id[0]);
        x += strlen(CONSOLE_CMDS[lc->cmd_suggerees[i]].id[0])*GLIMAGIMP_LARGEUR_POLICE/(float)(largeur_ecran);

        fixeCouleur(1.f, .3f, .0f);
        writeString(x, 1.f-(3+i)*h+y_offset, " (");
        x += 2*GLIMAGIMP_LARGEUR_POLICE/(float)(largeur_ecran);

        fixeCouleur(0.f, 1.f, .0f);
        writeString(x, 1.f-(3+i)*h+y_offset, (char*) CONSOLE_CMDS[lc->cmd_suggerees[i]].id[1]);
        x += strlen(CONSOLE_CMDS[lc->cmd_suggerees[i]].id[1])*GLIMAGIMP_LARGEUR_POLICE/(float)(largeur_ecran);

        fixeCouleur(1.f, .3f, .0f);
        writeString(x, 1.f-(3+i)*h+y_offset, ") - ");
        x += 4*GLIMAGIMP_LARGEUR_POLICE/(float)(largeur_ecran);

        fixeCouleur(1.f, 1.f, 0.f);
        writeString(x, 1.f-(3+i)*h+y_offset, (char*) CONSOLE_CMDS[lc->cmd_suggerees[i]].details);
    }
    if(lc->tampon[0]) {
        fixeCouleur(.9f, .9f, .9f);
        writeString(0.f, 1.f-2.f*h+y_offset, lc->tampon);
    } else {
        fixeCouleur(.6f, .6f, .6f);
        writeString(0.f, 1.f-2.f*h+y_offset, "Tapez quelques lettres. ([ENTREE]:executer - [ECHAP]:sortir)");
    }
}

void Imagimp_fonctionDessin(Imagimp *imagimp) {
#define img (imagimp->calques.rendu)
#define ihm_l (imagimp->largeur_ihm)
    dessinerLesHistogrammes(imagimp, (img.h-imagimp->hauteur_lignecmd)/16.f);
    float h_ligne = (imagimp->hauteur_lignecmd)/(float)img.h;
    dessinerConsole(&imagimp->console, h_ligne, 
        img.l, img.l+ihm_l, imagimp->fonction_clavier == Imagimp_fonctionClavierTexte);
#undef img
#undef ihm_l
}
