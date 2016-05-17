#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <GL/glut.h>
#include "glimagimp/interface.h"
#include "glimagimp/outils.h"
#include "Imagimp.h"
#include "ImageRVB.h"

static void Imagimp_usage(const char *nom_prog) {
    fprintf(stderr, "\n--- Usages possibles ---\n\n"
                    "Lancer avec une image PPM :\n    %s <img.ppm> [<commande>[:<param1>]*]*\n"
                    "Lancer sans image :\n    %s :<largeur>x<hauteur> [<commande>[:<param1>]*]*\n\n"
                    "Dans tous les cas, la taille du canevas est fixée par le premier argument.\n"
                    "Une taille de 512x512 est recommandée.\n"
                    "Les commandes sont résolues par recherche approximative :\n"
                    "Par exemple, 'PIA', 'SE' voire 'EPI' vont au final correspondre à 'SEPIA'.\n"
                    "La console de l'IHM permet de découvrir les commandes et d'expérimenter.\n\n"
                    "Exemples d'utilisation :\n"
                    "   %s :512x512 rc:255:128:56 \n"
                    "   %s :512x512 COULEUR:255:128:56 \n"
                    "   %s images/Mire_HSV.512.ppm SEPIA ADDLUM:230 INVERT DIMCON:50\n"
                    "   %s images/Mire_HSV.512.ppm SE ADDL:230 INV DIMC:50\n"
                    "   %s :400x320 IM_1:images/Phoenix.512.ppm IM_1:images/Clown.256.ppm e:export/clownix.ppm q\n",
                    nom_prog, nom_prog, nom_prog, nom_prog, nom_prog, nom_prog, nom_prog);
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
        if(!ImageRVB_allouer(&chargee, l, h)) {
            fputs("N'a pas pu allouer une image.", stderr);
            return false;
        }
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
    imagimp->vue_export = true;

    if(!PileCalques_allouer(&imagimp->calques, l, h)) {
        fputs("N'a pas pu allouer la pile de calques.", stderr);
        return false;
    }
    /* ImageRVB_remplirEchiquier(&imagimp->calques.virtuel, 16, 64, 150); */
    ImageRVB_remplirRVB(&imagimp->calques.virtuel, 255, 255, 255);
    ImageRVB_desallouer(&imagimp->calques.courant->img_source);
    imagimp->calques.courant->img_source.rvb = chargee.rvb;

    memset(&imagimp->console, 0, sizeof(Console));
    size_t i, j;
    for(i=2 ; i<argc ; ++i) {
        Console_effacerEntree(&imagimp->console);
        size_t len = strlen(argv[i]);
        for(j=0 ; j<len ; ++j)
            Console_insererCaractere(&imagimp->console, argv[i][j]==':' ? ' ' : argv[i][j]);
        Console_executer(&imagimp->console, imagimp);
    }
    strncpy(imagimp->console.reponse, "VIMagimp | Lucas DUSSOUCHAUD - Yoan LECOQ | promo IMAC 2018", CONSOLE_MAX_REPONSE);


    Calque_recalculer(imagimp->calques.courant);
    PileCalques_recalculer(&imagimp->calques);
#define img (imagimp->calques.rendu_gl)
    initGLIMAGIMP_IHM(img.l, img.h, img.rvb, img.l+imagimp->largeur_ihm, img.h+2*imagimp->hauteur_lignecmd);
#undef img
    return true;
}

/* On peut pas prendre un const Imagimp parce que actualiseImage() ne prend
 * pas un tableau const. Shame. */
void Imagimp_actualiserAffichageCanevas(Imagimp *imagimp) {
    if(imagimp->vue_export)
        actualiseImage(imagimp->calques.rendu_gl.rvb);
    else {
        ImageRVB_copierSymetrieVerticale(&imagimp->calques.rendu_vuesource_gl, &imagimp->calques.courant->img_source);
        actualiseImage(imagimp->calques.rendu_vuesource_gl.rvb);
        ImageRVB_histogrammeRVB(&imagimp->calques.courant->img_source, &imagimp->calques.histogramme_vuesource_rvb);
        ImageRVB_histogrammeR  (&imagimp->calques.courant->img_source, &imagimp->calques.histogramme_vuesource_r);
        ImageRVB_histogrammeV  (&imagimp->calques.courant->img_source, &imagimp->calques.histogramme_vuesource_v);
        ImageRVB_histogrammeB  (&imagimp->calques.courant->img_source, &imagimp->calques.histogramme_vuesource_b);
    }
}

void Imagimp_fonctionClavierTexte(Imagimp *imagimp, unsigned char ascii, int x, int y) {
    switch(ascii) {
    case '\t': Console_completer(&imagimp->console); break;
    case '\n':
    case '\r':
        Console_executer(&imagimp->console, imagimp);
    case '\b': 
        if(ascii=='\b' && imagimp->console.tampon[0]) { 
            Console_enleverCaractere(&imagimp->console);
            break;
        }
    case '\033':
        Console_effacerEntree(&imagimp->console);
        imagimp->fonction_clavier = Imagimp_fonctionClavier;
        imagimp->fonction_clavier_special = Imagimp_fonctionClavierSpecial;
        break;
    default:
        if(isprint(ascii))
            Console_insererCaractere(&imagimp->console, ascii);
        break;
    }
}

void Imagimp_fonctionClavier(Imagimp *imagimp, unsigned char ascii, int x, int y) {
    /* printf("Touche : '%c' (souris: %d, %d)\n", ascii, x, y); */
    switch(ascii) {
    default:
    case '\t':
    case ':':
        Console_effacerEntree(&imagimp->console);
        imagimp->fonction_clavier = Imagimp_fonctionClavierTexte;
        if(ascii!='\t' && ascii!=':')
            Imagimp_fonctionClavierTexte(imagimp, ascii, x, y);
        break;
    }
}

void Imagimp_fonctionClavierSpecial(Imagimp *imagimp, int touche, int x, int y) {
    /*printf("Touche spéciale : %d (souris: %d, %d)\n", touche, x, y);*/
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

void Imagimp_fonctionSouris(Imagimp *imagimp, int bouton, int appuye, int x, int y) {
    /*
    printf("clic: %d (%s) vers %d, %d\n", 
           bouton, appuye ? "pressé" : "laché", x, y);
    */
}



/* Dans toutes les fonctions de dessin de GLimagimp,
 * le repère est comme suit :
 *  
 * 1 ^
 *   |
 *   |
 *   |
 *   .------> 
 * 0        1
 *
 * Dans ce repère, la hauteur de la fenetre est 1,
 * tout comme sa largeur.
 */


static void dessinerHistogramme(const Histogramme *histo, float x, float y, float l, float h) {
    const float pas = l/256.f;
    size_t i;
    for(i=0 ; i<256 ; ++i)
        drawLigne(x+i*pas, y-h,
                  x+i*pas, y-h+h*histo->donnees[i]/(float)histo->max);

}
static void dessinerLesHistogrammes(const Imagimp *imagimp, float histo_h) {
    const Histogramme * const histos[4] = {
        (imagimp->vue_export ? &imagimp->calques.histogramme_rendu_rvb : &imagimp->calques.histogramme_vuesource_rvb),
        (imagimp->vue_export ? &imagimp->calques.histogramme_rendu_r : &imagimp->calques.histogramme_vuesource_r),
        (imagimp->vue_export ? &imagimp->calques.histogramme_rendu_v : &imagimp->calques.histogramme_vuesource_v),
        (imagimp->vue_export ? &imagimp->calques.histogramme_rendu_b : &imagimp->calques.histogramme_vuesource_b)
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
        fixeCouleur(.0f, 1.f, .1f);
    writeString(0.f, 1.f-h+y_offset, lc->reponse);

    fixeCouleur(.9f, .9f, .9f);
    if(!activee) {
        writeString(0.f, 1.f-2.f*h+y_offset, "Entrer une commande avec [TAB] ou ':'.");
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
        fixeCouleur(.6f, .6f, .6f);
        writeString(0.f, 1.f-2.f*h+y_offset, lc->tampon_suggestion);
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
