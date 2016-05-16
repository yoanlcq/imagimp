#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Imagimp.h"

/* J'espère que t'as un écran large */

static void nonImplementee(Imagimp *imagimp, int argc, const char * const * argv) {
    snprintf(imagimp->console.reponse, CONSOLE_MAX_REPONSE, "%s: Cette commande n'est pas encore implementee.", argv[0]);
    imagimp->console.reponse_rouge = true;
}
static void nonReconnue(Imagimp *imagimp, int argc, const char * const *argv) {
    strncpy(imagimp->console.reponse, "Commande non reconnue.", CONSOLE_MAX_REPONSE);
    imagimp->console.reponse_rouge = true;
}
static void auRevoir(Imagimp *imagimp, int argc, const char * const *argv) {
    PileCalques_desallouerTout(&imagimp->calques);
    exit(EXIT_SUCCESS);
}

const ConsoleCmd CONSOLE_CMDS[CONSOLE_MAX_CMDS] = {
    { nonImplementee, {"cn"   , "CAL_1"},            "Calque, Nouveau", ""},
    { nonImplementee, {"cs"   , "CAL_5"},            "Calque, Supprimer", ""},
    { nonImplementee, {"cc"   , "CAL_2"},            "Calque, Changer (Clavier : fleches haut et bas)", "<ecart>"},
    { nonImplementee, {"co"   , "CAL_3"},            "Calque, Opacite (Clavier : fleches gauche et droite)", "[0;1]"},
    { nonImplementee, {"cmn"  , "CAL_4_NORMAL"},     "Calque, Melange Normal", ""},
    { nonImplementee, {"cma"  , "CAL_4_ADD"},        "Calque, Melange Addition", ""},
    { nonImplementee, {"cmm"  , "CAL_4_MUL"},        "Calque, Melange Multiplication", ""},
    { nonImplementee, {"cd"   , "CAL_EXT_DEPLACER"}, "Calque, Deplacer", "<ecart>"},
    { nonImplementee, {"crc"  , "CAL_EXT_COULEUR"},  "Calque, Remplir par une Couleur", "[0;255] [0;255] [0;255]"},
    { nonImplementee, {"cri"  , "IM_1"},             "Calque, Remplir par une Image PPM", "images/Phoenix.512.ppm"},
    { nonImplementee, {"clni" , "LUT_1_INVERT"},     "Calque, LUT, Nouvelle, Inversion", ""},
    { nonImplementee, {"clns" , "LUT_1_SEPIA"},      "Calque, LUT, Nouvelle, Sepia", ""},
    { nonImplementee, {"clnal", "LUT_1_ADDLUM"},     "Calque, LUT, Nouvelle, Augmentation Luminosite", ""},
    { nonImplementee, {"clndl", "LUT_1_DIMLUM"},     "Calque, LUT, Nouvelle, Diminution Luminosite", ""},
    { nonImplementee, {"clnac", "LUT_1_ADDCON"},     "Calque, LUT, Nouvelle, Augmentation Contraste", ""},
    { nonImplementee, {"clndc", "LUT_1_DIMCON"},     "Calque, LUT, Nouvelle, Diminution Contraste", ""},
    { nonImplementee, {"clnar", "LUT_1_EXT_ADDR"},   "Calque, LUT, Nouvelle, Augmentation Rouge", ""},
    { nonImplementee, {"clnav", "LUT_1_EXT_ADDV"},   "Calque, LUT, Nouvelle, Augmentation Vert", ""},
    { nonImplementee, {"clnab", "LUT_1_EXT_ADDB"},   "Calque, LUT, Nouvelle, Augmentation Bleu", ""},
    { nonImplementee, {"clndr", "LUT_1_EXT_DIMR"},   "Calque, LUT, Nouvelle, Diminution Rouge", ""},
    { nonImplementee, {"clndv", "LUT_1_EXT_DIMV"},   "Calque, LUT, Nouvelle, Diminution Vert", ""},
    { nonImplementee, {"clndb", "LUT_1_EXT_DIMB"},   "Calque, LUT, Nouvelle, Diminution Bleu", ""},
    { nonImplementee, {"clap" , "LUT_2"},            "Calque, LUT, Appliquer la Premiere", ""},
    { nonImplementee, {"clsd" , "LUT_3"},            "Calque, LUT, Supprimer la Derniere", ""},
    { nonImplementee, {"clp"  , "LUT_EXT_PARAMS"},   "Calque, LUT, Parametres (Clavier : '<' et '>')", ""},
    { nonImplementee, {"v"    , "IHM_1"},            "Vue (cycle entre 'export' et 'image source du calque')", ""},
    { nonImplementee, {"eh"   , "IM_EXT_HISTG"},     "Exporter Histogrammes (PPM)", ""},
    { nonImplementee, {"e"    , "IM_2"},             "Exporter (PPM)", ""},
    { auRevoir, {"q"    , "IHM_4"},            "Quitter", ""},
    {NULL, {NULL, NULL}, NULL, NULL} /* Indique la fin du tableau */
};

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
}
void Console_effacerEntree(Console *lc) {
    lc->curseur = 0;
    memset(lc->tampon, 0, CONSOLE_MAX_TAMPON);
    Console_suggerer(lc);
}
void Console_completer(Console *lc) {
    /* TODO, quand tu veux */
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

    if(lc->nb_cmd_suggerees <= 0) {
        char old = *fin;
        *fin = '\0';
        nonReconnue(imagimp, 1, (const char* const*) &cmdid);
        *fin = old;
        return;
    }

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

