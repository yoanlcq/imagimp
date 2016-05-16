#ifndef LUT_H
#define LUT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct LUT LUT;
typedef void (*FonctionLUT)(LUT *lut);
struct LUT {
    uint8_t r[256];
    uint8_t v[256];
    uint8_t b[256];
    FonctionLUT fonction; /* Sert à deux choses : 
                1) identifier le type de la LUT ;
                2) recalculer les valeurs des tables, d'après param1. */
    uint8_t param1; /* luminosité ou contraste selon le cas. */
    LUT *precedente, *suivante;
};
typedef struct ListeLUTs ListeLUTs;
struct ListeLUTs {
    LUT *premiere, *derniere;
};

/* Utilisation correcte :
 * ListeLUTs liste;
 * ListeLUTs_allouer(&liste);
 * ListeLUTs_ajouterDerniere(&liste);
 * // Modifier liste->derniere....
 * ListeLUTs_desallouer(&liste);
 */

bool ListeLUTs_allouer(ListeLUTs *liste);
void ListeLUTs_desallouer(ListeLUTs *liste);
bool ListeLUTs_ajouterDerniere(ListeLUTs *liste);
void ListeLUTs_retirerDerniere(ListeLUTs *liste);
void ListeLUTs_retirerPremiere(ListeLUTs *liste);

void LUT_inversion(LUT *lut);
void LUT_augmentationLuminosite(LUT *lut);
void LUT_diminutionLuminosite(LUT *lut);
void LUT_augmentationContraste(LUT *lut);
void LUT_diminutionContraste(LUT *lut);
void LUT_sepia(LUT *lut);
void LUT_augmentationR(LUT *lut);
void LUT_augmentationV(LUT *lut);
void LUT_augmentationB(LUT *lut);
void LUT_diminutionR(LUT *lut);
void LUT_diminutionV(LUT *lut);
void LUT_diminutionB(LUT *lut);

#endif /* LUT_H */
