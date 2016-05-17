#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>
#include "Imagimp.h"

#define CONSOLE_MAX_TAMPON  512 /* Va dépasser ça. */
#define CONSOLE_MAX_REPONSE 512
#define CONSOLE_MAX_CMDS 64 /* Attention, pas forcément le nombre exact de commandes. */
#define CONSOLE_CMD_MAX_ALIAS 2

typedef struct {
    void (*fonction)(Imagimp *imagimp, int argc, const char* const* argv);
    const char *id[CONSOLE_CMD_MAX_ALIAS];
    const char *details;
    const char *params_suggeres;
} ConsoleCmd;

extern const ConsoleCmd CONSOLE_CMDS[CONSOLE_MAX_CMDS];

struct Console {
    size_t curseur;
    char tampon[CONSOLE_MAX_TAMPON];
    char tampon_suggestion[CONSOLE_MAX_TAMPON];
    char reponse[CONSOLE_MAX_REPONSE];
    bool reponse_rouge;
    uint8_t cmd_suggerees[CONSOLE_MAX_CMDS]; /* tableau d'index, triés. */
    uint8_t nb_cmd_suggerees;
};
typedef struct Console Console;

void Console_effacerEntree(Console *lc);
void Console_completer(Console *lc);
void Console_executer(Console *lc, Imagimp *imagimp);
void Console_enleverCaractere(Console *lc);
void Console_insererCaractere(Console *lc, char ascii);

#endif /* CONSOLE_H */
