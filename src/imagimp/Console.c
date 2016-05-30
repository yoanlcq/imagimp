#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Imagimp.h"

/* J'espère que t'as un écran large */

/* Les fonctions static qui suivent sont les callbacks pour chaque commande.
 * Je conseille de regarder le tableau CONSOLE_CMDS plus bas qui décrit un peu chaque commande. */

static void nonReconnue(Imagimp *imagimp, int argc, const char * const *argv) {
    strncpy(imagimp->console.reponse, "Commande non reconnue.", CONSOLE_MAX_REPONSE);
    imagimp->console.reponse_rouge = true;
}
static void cmd_q(Imagimp *imagimp, int argc, const char * const *argv) {
    PileCalques_desallouerTout(&imagimp->calques);
    exit(EXIT_SUCCESS);
}
/* Celle-ci n'est pas une commande, mais tellement de commandes font ces opérations
 * que ça mérite d'être une fonction. */
static void recalculerCalquePuisPilePuisAfficher(Imagimp *imagimp) {
    Calque_recalculer(imagimp->calques.courant);
    PileCalques_recalculer(&imagimp->calques);
    Imagimp_actualiserAffichageCanevas(imagimp);
}
static void cmd_cn(Imagimp *imagimp, int argc, const char * const *argv) {
    if(!PileCalques_ajouterCalqueVierge(&imagimp->calques)) {
        strncpy(imagimp->console.reponse, "N'a pas pu allouer un nouveau calque.", CONSOLE_MAX_REPONSE);
        imagimp->console.reponse_rouge = true;
        return;
    }
    recalculerCalquePuisPilePuisAfficher(imagimp);
    strncpy(imagimp->console.reponse,
            "Voici un nouveau calque. "
            "Vous pouvez le remplir avec 'crc' ou 'cri'.",
            CONSOLE_MAX_REPONSE);
}
static void cmd_cs(Imagimp *imagimp, int argc, const char * const *argv) {
    if(!PileCalques_supprimerCalqueCourant(&imagimp->calques)) {
        strncpy(imagimp->console.reponse, "Vous ne pouvez pas supprimer le dernier calque restant.", CONSOLE_MAX_REPONSE);
        imagimp->console.reponse_rouge = true;
        return;
    }
    PileCalques_recalculer(&imagimp->calques);
    Imagimp_actualiserAffichageCanevas(imagimp);
    imagimp->console.reponse[0] = '\0';
}
static void cmd_cc(Imagimp *imagimp, int argc, const char * const *argv) {
    if(argc <= 1) {
        strncpy(imagimp->console.reponse, "Une valeur d'ecart est requise.", CONSOLE_MAX_REPONSE);
        imagimp->console.reponse_rouge = true;
        return;
    }
    int ecart = strtol(argv[1], NULL, 0);
    Calque *old = imagimp->calques.courant;
    if(ecart > 0) {
        for( ; imagimp->calques.courant->au_dessus && ecart > 0 ; --ecart)
            imagimp->calques.courant = imagimp->calques.courant->au_dessus;
    } else if(ecart < 0) {
        for( ; imagimp->calques.courant->en_dessous && ecart < 0 ; ++ecart)
            imagimp->calques.courant = imagimp->calques.courant->en_dessous;
    }
    if(!imagimp->vue_export)
        Imagimp_actualiserAffichageCanevas(imagimp);
        snprintf(imagimp->console.reponse, CONSOLE_MAX_REPONSE, "le calque courant %s change.",
                imagimp->calques.courant != old ? "a" : "n'a pas");
}
static void cmd_co(Imagimp *imagimp, int argc, const char * const *argv) {
    if(argc <= 1) {
        strncpy(imagimp->console.reponse, "La valeur d'opacite est requise.", CONSOLE_MAX_REPONSE);
        imagimp->console.reponse_rouge = true;
        return;
    }
    float alpha = strtof(argv[1], NULL);
         if(alpha<0.f)   alpha = 0.f;
    else if(alpha>100.f) alpha = 100.f;
    alpha /= 100.f;
    imagimp->calques.courant->opacite = alpha;
    recalculerCalquePuisPilePuisAfficher(imagimp);
    snprintf(imagimp->console.reponse, CONSOLE_MAX_REPONSE, "Opacite : %.2f%%", alpha*100.f);
}
static void cmd_cm_helper(Imagimp *imagimp, int argc, const char *const* argv,
                         FonctionMelange melange) {
    imagimp->calques.courant->melange = melange;
    recalculerCalquePuisPilePuisAfficher(imagimp);
    imagimp->console.reponse[0] = '\0';
}
static void cmd_cmn(Imagimp *imagimp, int argc, const char * const *argv) {
    cmd_cm_helper(imagimp, argc, argv, Melange_normal);
}
static void cmd_cma(Imagimp *imagimp, int argc, const char * const *argv) {
    cmd_cm_helper(imagimp, argc, argv, Melange_addition);
}
static void cmd_cmm(Imagimp *imagimp, int argc, const char * const *argv) {
    cmd_cm_helper(imagimp, argc, argv, Melange_multiplication);
}
static void cmd_cme(Imagimp *imagimp, int argc, const char * const *argv) {
    cmd_cm_helper(imagimp, argc, argv, Melange_ecran);
}
static void cmd_crc(Imagimp *imagimp, int argc, const char * const *argv) {
    if(argc <= 3) {
        strncpy(imagimp->console.reponse, "Trois valeurs : rouge, vert et bleu, sont requises.", CONSOLE_MAX_REPONSE);
        imagimp->console.reponse_rouge = true;
        return;
    }
    uint8_t r,v,b;
    r = strtoul(argv[1], NULL, 0);
    v = strtoul(argv[2], NULL, 0);
    b = strtoul(argv[3], NULL, 0);
    ImageRVB_remplirRVB(&imagimp->calques.courant->img_source, r, v, b);
    recalculerCalquePuisPilePuisAfficher(imagimp);
    imagimp->console.reponse[0] = '\0';
}
static void cmd_cri(Imagimp *imagimp, int argc, const char * const *argv) {
    if(argc <= 1) {
        strncpy(imagimp->console.reponse, "Le nom du fichier PPM est requis.", CONSOLE_MAX_REPONSE);
        imagimp->console.reponse_rouge = true;
        return;
    }
    ImageRVB ppm;
    if(!ImageRVB_importerPPM(&ppm, argv[1])) {
        snprintf(imagimp->console.reponse, CONSOLE_MAX_REPONSE, "N'a pas pu importer '%s'.", argv[1]);
        imagimp->console.reponse_rouge = true;
        return;
    }
    ImageRVB_copier(&imagimp->calques.courant->img_source, &ppm);
    ImageRVB_desallouer(&ppm);
    recalculerCalquePuisPilePuisAfficher(imagimp);
    imagimp->console.reponse[0] = '\0';
}
static void cmd_cln_helper(Imagimp *imagimp, int argc, const char *const* argv, void (*lut_func)(LUT *lut)) {
    if(!ListeLUTs_ajouterDerniere(&imagimp->calques.courant->luts)) {
        strncpy(imagimp->console.reponse, "N'a pas pu allouer une LUT.", CONSOLE_MAX_REPONSE);
        imagimp->console.reponse_rouge = true;
        return;
    }
    LUT *lut = imagimp->calques.courant->luts.derniere;
    if(argc > 1)
        lut->param1 = strtoul(argv[1], NULL, 0);
    else
        lut->param1 = 200;
    lut->fonction = lut_func;
    lut->fonction(lut);
    recalculerCalquePuisPilePuisAfficher(imagimp);
    strncpy(imagimp->console.reponse, "LUT ajoutee en fin de liste. Changer ses parametres avec 'clpd'.", CONSOLE_MAX_REPONSE);
}
static void cmd_clni(Imagimp *imagimp, int argc, const char * const *argv)   { cmd_cln_helper(imagimp, argc, argv, LUT_inversion); }
static void cmd_clns(Imagimp *imagimp, int argc, const char * const *argv)   { cmd_cln_helper(imagimp, argc, argv, LUT_sepia); }
static void cmd_clnal(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_augmentationLuminosite); }
static void cmd_clndl(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_diminutionLuminosite); }
static void cmd_clnac(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_augmentationContraste); }
static void cmd_clndc(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_diminutionContraste); }
static void cmd_clnar(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_augmentationR); }
static void cmd_clnav(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_augmentationV); }
static void cmd_clnab(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_augmentationB); }
static void cmd_clndr(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_diminutionR); }
static void cmd_clndv(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_diminutionV); }
static void cmd_clndb(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_diminutionB); }
static void cmd_clnexp(Imagimp *imagimp, int argc, const char * const *argv) { cmd_cln_helper(imagimp, argc, argv, LUT_exp); }
static void cmd_clnln(Imagimp *imagimp, int argc, const char * const *argv)  { cmd_cln_helper(imagimp, argc, argv, LUT_ln); }
//static void cmd_clng(Imagimp *imagimp, int argc, const char * const *argv)   { cmd_cln_helper(imagimp, argc, argv, LUT_gamma); }
//static void cmd_clncos(Imagimp *imagimp, int argc, const char * const *argv) { cmd_cln_helper(imagimp, argc, argv, LUT_cos); }
//static void cmd_clnsin(Imagimp *imagimp, int argc, const char * const *argv) { cmd_cln_helper(imagimp, argc, argv, LUT_sin); }
static void cmd_clap(Imagimp *imagimp, int argc, const char * const *argv) { 
    Calque_appliquerPremiereLUT(imagimp->calques.courant);
    recalculerCalquePuisPilePuisAfficher(imagimp);
    strncpy(imagimp->console.reponse, "LUT appliquee.", CONSOLE_MAX_REPONSE);
}
static void cmd_clsd(Imagimp *imagimp, int argc, const char * const *argv) { 
    ListeLUTs_retirerDerniere(&imagimp->calques.courant->luts);
    recalculerCalquePuisPilePuisAfficher(imagimp);
    strncpy(imagimp->console.reponse, "La derniere LUT a ete supprimee.", CONSOLE_MAX_REPONSE);
}
static void cmd_clpd(Imagimp *imagimp, int argc, const char * const *argv) {
    if(argc <= 1) {
        strncpy(imagimp->console.reponse, "Une valeur est requise.", CONSOLE_MAX_REPONSE);
        imagimp->console.reponse_rouge = true;
        return;
    }
    LUT *lut = imagimp->calques.courant->luts.derniere;
    lut->param1 = strtoul(argv[1], NULL, 0);
    lut->fonction(lut);
    recalculerCalquePuisPilePuisAfficher(imagimp);
    imagimp->console.reponse[0] = '\0';

}
static void cmd_v(Imagimp *imagimp, int argc, const char * const *argv) { 
    imagimp->vue_export = !imagimp->vue_export;
    Imagimp_actualiserAffichageCanevas(imagimp);
    snprintf(imagimp->console.reponse, 
            CONSOLE_MAX_REPONSE, 
            "Visualisation %s.", 
            imagimp->vue_export ? "de l'image finale" : "des pixels originaux du calque");
}
static void cmd_e(Imagimp *imagimp, int argc, const char * const *argv) {
    const char *nom_fichier = argc>1 ? argv[1] : "sans_titre.ppm";
    if(!ImageRVB_exporterPPM(&imagimp->calques.rendu, nom_fichier)) {
        snprintf(imagimp->console.reponse, CONSOLE_MAX_REPONSE, "N'a pas pu exporter vers '%s'.", nom_fichier);
        imagimp->console.reponse_rouge = true;
        return;
    }
    snprintf(imagimp->console.reponse, CONSOLE_MAX_REPONSE, "Image exportee vers '%s'.", nom_fichier);
    printf("Image exportee vers '%s'.\n", nom_fichier);
}

const ConsoleCmd CONSOLE_CMDS[CONSOLE_MAX_CMDS] = {
{ cmd_cn,     {"cn"    , "CAL_1"},           "Calque, Nouveau", ""},
{ cmd_cs,     {"cs"    , "CAL_5"},           "Calque, Supprimer", ""},
{ cmd_cc,     {"cc"    , "CAL_2"},           "Calque, Changer de calque courant", "<position_relative_au_calque_courant>"},
{ cmd_co,     {"co"    , "CAL_3"},           "Calque, Opacite", "[0;100]"},
{ cmd_cmn,    {"cmn"   , "CAL_4_NORMAL"},    "Calque, Melange Normal", ""},
{ cmd_cma,    {"cma"   , "CAL_4_ADD"},       "Calque, Melange Addition", ""},
{ cmd_cmm,    {"cmm"   , "CAL_4_MUL"},       "Calque, Melange Multiplication", ""},
{ cmd_cme,    {"cme"   , "CAL_4_ECRAN"},     "Calque, Melange \"Ecran\"", ""},
{ cmd_crc,    {"crc"   , "CAL_EXT_COULEUR"}, "Calque, Remplir par une Couleur", "[0;255] [0;255] [0;255]"},
{ cmd_cri,    {"cri"   , "IM_1"},            "Calque, Remplir par une Image PPM", "images/Phoenix.512.ppm"},
{ cmd_clni,   {"clni"  , "INVERT"},          "Calque, LUT, Nouvelle, Inversion", ""},
{ cmd_clns,   {"clns"  , "SEPIA"},           "Calque, LUT, Nouvelle, Sepia", ""},
{ cmd_clnal,  {"clnal" , "ADDLUM"},          "Calque, LUT, Nouvelle, Augmentation Luminosite", "[0;255]"},
{ cmd_clndl,  {"clndl" , "DIMLUM"},          "Calque, LUT, Nouvelle, Diminution Luminosite", "[0;255]"},
{ cmd_clnac,  {"clnac" , "ADDCON"},          "Calque, LUT, Nouvelle, Augmentation Contraste", "[0;255]"},
{ cmd_clndc,  {"clndc" , "DIMCON"},          "Calque, LUT, Nouvelle, Diminution Contraste", "[0;255]"},
{ cmd_clnar,  {"clnar" , "ADDR"},            "Calque, LUT, Nouvelle, Augmentation Rouge", "[0;255]"},
{ cmd_clnav,  {"clnav" , "ADDV"},            "Calque, LUT, Nouvelle, Augmentation Vert", "[0;255]"},
{ cmd_clnab,  {"clnab" , "ADDB"},            "Calque, LUT, Nouvelle, Augmentation Bleu", "[0;255]"},
{ cmd_clndr,  {"clndr" , "DIMR"},            "Calque, LUT, Nouvelle, Diminution Rouge", "[0;255]"},
{ cmd_clndv,  {"clndv" , "DIMV"},            "Calque, LUT, Nouvelle, Diminution Vert", "[0;255]"},
{ cmd_clndb,  {"clndb" , "DIMB"},            "Calque, LUT, Nouvelle, Diminution Bleu", "[0;255]"},
{ cmd_clnexp, {"clnexp", "EXP"},             "Calque, LUT, Nouvelle, Exponentielle", "[0;255]"},
{ cmd_clnln,  {"clnln" , "LN"},              "Calque, LUT, Nouvelle, Logarithme Neperien", "[0;255]"},
//{ cmd_clng,   {"clng"  , "GAMMA"},           "Calque, LUT, Nouvelle, Gamma", "[0;255]"},
//{ cmd_clncos, {"clncos", "COS"},             "Calque, LUT, Nouvelle, Cosinus", "[0;255]"},
//{ cmd_clnsin, {"clnsin", "SIN"},             "Calque, LUT, Nouvelle, Sinus", "[0;255]"},
{ cmd_clap,   {"clap"  , "LUT_2"},           "Calque, LUT, Appliquer la Premiere de la liste", ""},
{ cmd_clsd,   {"clsd"  , "LUT_3"},           "Calque, LUT, Supprimer la Derniere de la liste", ""},
{ cmd_clpd,   {"clpd"  , "LUT_EXT_PARAMS"},  "Calque, LUT, changer les Parametres de la Derniere", "[0:255]"},
{ cmd_v,      {"v"     , "IHM_1"},           "Vue (cycle entre 'export' et 'image source du calque')", ""},
{ cmd_e,      {"e"     , "IM_2"},            "Exporter (PPM)", "sans_titre.ppm"},
{ cmd_q,      {"q"     , "IHM_4"},           "Quitter", ""},
{NULL, {NULL, NULL}, NULL, NULL} /* Indique la fin du tableau */
};

static void Console_suggererTampon(Console *lc) {
    if(lc->nb_cmd_suggerees <= 0) {
        lc->tampon_suggestion[0] = '\0';
        return;
    }
    const ConsoleCmd *cmd = CONSOLE_CMDS + lc->cmd_suggerees[0];

    char *cmdid = lc->tampon;
    while(isspace(*cmdid) && *cmdid) 
        ++cmdid;

    size_t nargs;
    for(nargs=0 ; *cmdid ; ++nargs) {
        while(!isspace(*cmdid) && *cmdid) 
            ++cmdid;
        while(isspace(*cmdid) && *cmdid) 
            ++cmdid;
    }
    if(!nargs) nargs = 1;

    const char *sugg = cmd->params_suggeres;
    while(isspace(*sugg) && *sugg) 
        ++sugg;
    for(--nargs; *sugg && nargs>0 ; --nargs) {
        while(!isspace(*sugg) && *sugg) 
            ++sugg;
        while(isspace(*sugg) && *sugg) 
            ++sugg;
    }
    size_t len = strlen(lc->tampon);
    if(!isspace(lc->tampon[len-1]))
        ++len;
    memset(lc->tampon_suggestion, ' ', len);
    strncpy(lc->tampon_suggestion+len, sugg, CONSOLE_MAX_TAMPON-len);
}

static void Console_suggerer(Console *lc) {
    char *cmdid = lc->tampon;
    while(isspace(*cmdid) && *cmdid) 
        ++cmdid;
    if(!(*cmdid)) {
        size_t i;
        const ConsoleCmd *cmd;
        for(cmd=CONSOLE_CMDS, i=0 ; cmd->id[0] ; ++cmd, ++i)
            lc->cmd_suggerees[i] = i;
        lc->nb_cmd_suggerees = i;
        return;
    }

    lc->nb_cmd_suggerees = 0;

    char *fin = cmdid;
    while(!isspace(*fin) && *fin) 
        ++fin;
    char old = *fin;
    *fin = '\0'; /* le temps des comparaisons */

    size_t i;
    const ConsoleCmd *cmd;
    for(cmd=CONSOLE_CMDS, i=0 ; cmd->id[0] ; ++cmd, ++i) {
        if(strstr(cmd->id[0], cmdid) || strstr(cmd->id[1], cmdid)) {
            lc->cmd_suggerees[lc->nb_cmd_suggerees] = i;
            ++(lc->nb_cmd_suggerees);
        }
    }
    /* On refait une passe pour mettre en priorité la commande
     * qui matche exactement. C'est juste un échange d'index. */
    for(i=0 ; i<lc->nb_cmd_suggerees ; ++i) {
        if(!strcmp(CONSOLE_CMDS[lc->cmd_suggerees[i]].id[0], cmdid)
        || !strcmp(CONSOLE_CMDS[lc->cmd_suggerees[i]].id[1], cmdid)) {
            uint8_t tmp = lc->cmd_suggerees[0];
            lc->cmd_suggerees[0] = lc->cmd_suggerees[i];
            lc->cmd_suggerees[i] = tmp;
            break;
        }
    }
    *fin = old;

    Console_suggererTampon(lc);
}
void Console_effacerEntree(Console *lc) {
    lc->curseur = 0;
    memset(lc->tampon, 0, CONSOLE_MAX_TAMPON);
    Console_suggerer(lc);
}
void Console_completer(Console *lc) {
    strncpy(lc->reponse, "Ouais non, on n'a pas encore l'auto-completion. x)", CONSOLE_MAX_REPONSE);
    lc->reponse_rouge = true;
}
void Console_executer(Console *lc, Imagimp *imagimp) {
    char *cmdid = lc->tampon;
    while(isspace(*cmdid) && *cmdid) 
        ++cmdid;

    char *fin = cmdid;
    while(!isspace(*fin) && *fin) 
        ++fin;

    if(cmdid==fin)
        return;

    memset(lc->reponse, 0, CONSOLE_MAX_REPONSE);

    if(lc->nb_cmd_suggerees <= 0) {
        char old = *fin;
        *fin = '\0';
        nonReconnue(imagimp, 1, (const char* const*) &cmdid);
        *fin = old;
        return;
    }

    imagimp->console.reponse_rouge = false;

    /* Attention : code moche. */
    char argv[4][256]; /* Flemme de faire un malloc. */
    strncpy(argv[0], CONSOLE_CMDS[lc->cmd_suggerees[0]].id[1], 256);
    int argc = 1;
    const char *arg_debut, *arg_fin = fin;
    size_t i;
    for(i=1 ; *arg_fin && i<4 ; ++i) {
        arg_debut = arg_fin;
        while(isspace(*arg_debut) && *arg_debut) 
            ++arg_debut;
        if(!(*arg_debut))
            break;
        arg_fin = arg_debut;
        while(!isspace(*arg_fin) && *arg_fin)
            ++arg_fin;
        size_t len = arg_fin-arg_debut < 255 ? arg_fin-arg_debut : 255;
        strncpy(argv[i], arg_debut, len);
        argv[i][len] = '\0';
        ++argc;
    }
    const char * const argv_pp[4] = {
        argv[0], argv[1], argv[2], argv[3]
    };
    CONSOLE_CMDS[lc->cmd_suggerees[0]].fonction(imagimp, argc, argv_pp);
    return;
}
void Console_enleverCaractere(Console *lc) {
    if(lc->curseur<=0)
        return;
    --(lc->curseur);
    char *ptr;
    ptr = lc->tampon + lc->curseur;
    memmove(ptr, ptr+1, strrchr(lc->tampon, '\0')-ptr);
    Console_suggerer(lc);
}
void Console_insererCaractere(Console *lc, char ascii) {
    if(lc->curseur >= CONSOLE_MAX_TAMPON-1)
        return;
    lc->tampon[lc->curseur] = ascii;
    ++(lc->curseur);
    Console_suggerer(lc);
}

