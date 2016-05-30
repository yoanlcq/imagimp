#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "LUT.h"

bool ListeLUTs_allouer(ListeLUTs *liste) {
    liste->premiere = liste->derniere = NULL;
    return true;
}
void ListeLUTs_desallouer(ListeLUTs *liste) {
    if(!liste->premiere)
        return;
    LUT *cur;
    for(cur=liste->premiere ; cur!=liste->derniere ; cur=cur->suivante)
        free(cur);
    free(cur);
}
bool ListeLUTs_ajouterDerniere(ListeLUTs *liste) { 
    LUT *nouvelle = calloc(1, sizeof(LUT));
    if(!nouvelle) {
        fputs("N'a pas pu allouer une LUT.", stderr);
        return false;
    }
    if(liste->derniere) {
        nouvelle->precedente = liste->derniere;
        liste->derniere->suivante = nouvelle;
        liste->derniere = nouvelle;
    } else
        liste->premiere = liste->derniere = nouvelle;

    liste->derniere->fonction = LUT_inversion;
    liste->derniere->param1 = 127;
    return true; 
}
void ListeLUTs_retirerDerniere(ListeLUTs *liste) {
    if(!liste->derniere)
        return;
    LUT *prec = liste->derniere->precedente;
    free(liste->derniere);
    liste->derniere = prec;
    if(liste->derniere)
        liste->derniere->suivante = NULL;
    else
        liste->derniere = liste->premiere = NULL;
}
void ListeLUTs_retirerPremiere(ListeLUTs *liste) {
    LUT *old = liste->premiere;
    liste->premiere = liste->premiere->suivante;
    if(!liste->premiere)
        liste->derniere = NULL;
    free(old);
}

/* Les fonctions suivantes n'ont pas de paramètres */

void LUT_inversion(LUT *lut) {
    size_t i, inv; /* Ne surtout pas les mettre comme uint8_t ici, sinon
                      la boucle va tourner indéfiniment. (i ne pourrait jamais 
                      valoir 256 car il n'y aurait pas assez de bits.) */
    for(i=0, inv=255 ; i<256 ; ++i, --inv)
        lut->r[i] = lut->v[i] = lut->b[i] = inv;
}
void LUT_sepia(LUT *lut) {
    /* Bêtement copiée depuis https://github.com/srhartono/Globulator/blob/master/ImageJ/luts/sepia.lut */
    const uint8_t sepia[3*256] = {
          0,  0, 0 ,
         43, 28, 4 ,
         57, 35, 5 ,
         65, 35, 6 ,
         66, 36, 7 ,
         67, 37, 8 ,
         68, 38, 9 ,
         69, 39, 10,
         70, 40, 11,
         71, 41, 12,
         72, 42, 13,
         73, 43, 14,
         74, 44, 15,
         75, 45, 16,
         76, 46, 17,
         77, 47, 18,
         78, 48, 19,
         79, 49, 20,
         80, 50, 21,
         81, 51, 22,
         82, 52, 23,
         83, 53, 24,
         83, 53, 24,
         84, 54, 25,
         85, 55, 26,
         86, 56, 27,
         87, 57, 28,
         88, 58, 29,
         89, 59, 30,
         90, 60, 31,
         91, 61, 32,
         92, 62, 33,
         93, 63, 34,
         94, 64, 35,
         95, 65, 36,
         96, 66, 37,
         97, 67, 38,
         98, 68, 39,
         99, 69, 40,
        100, 70, 41,
        101, 71, 42,
        102, 72, 43,
        103, 73, 44,
        104, 74, 45,
        105, 75, 46,
        106, 76, 47,
        107, 77, 48,
        107, 77, 48,
        108, 78, 49,
        109, 79, 50,
        110, 80, 51,
        111, 81, 52,
        112, 82, 53,
        113, 83, 54,
        114, 84, 55,
        115, 85, 56,
        116, 86, 57,
        117, 87, 58,
        118, 88, 59,
        119, 89, 60,
        120, 90, 61,
        121, 91, 62,
        122, 92, 63,
        123, 93, 64,
        124, 94, 65,
        125, 95, 66,
        125, 95, 66,
        126, 96, 67,
        127, 97, 68,
        128, 98, 69,
        129, 99, 70,
        130,100, 71,
        131,101, 72,
        132,102, 73,
        133,103, 74,
        134,104, 75,
        135,105, 76,
        136,106, 77,
        137,107, 78,
        138,108, 79,
        139,109, 80,
        140,110, 81,
        141,111, 82,
        142,112, 83,
        143,113, 84,
        144,114, 85,
        145,115, 86,
        146,116, 87,
        147,117, 88,
        148,118, 89,
        149,119, 90,
        150,120, 91,
        151,121, 92,
        152,122, 93,
        153,123, 94,
        154,124, 95,
        155,125, 96,
        156,126, 97,
        157,127, 98,
        158,128, 99,
        159,129,100,
        160,130,101,
        161,131,102,
        162,132,103,
        163,133,104,
        164,134,105,
        165,135,106,
        166,136,107,
        167,137,108,
        168,138,109,
        169,139,110,
        170,140,111,
        171,141,112,
        172,142,113,
        173,143,114,
        174,144,115,
        175,145,116,
        176,146,117,
        177,147,118,
        178,148,119,
        179,149,120,
        180,150,121,
        181,151,122,
        182,152,123,
        183,153,124,
        184,154,125,
        185,155,126,
        186,156,127,
        187,157,128,
        187,157,128,
        188,158,129,
        189,159,130,
        190,160,131,
        191,161,132,
        192,162,133,
        193,163,134,
        194,164,135,
        195,165,136,
        196,166,137,
        197,167,138,
        198,168,139,
        199,169,140,
        200,170,141,
        201,171,142,
        202,172,143,
        203,173,144,
        204,174,145,
        205,175,146,
        206,176,147,
        207,177,148,
        208,178,149,
        209,179,150,
        210,180,151,
        211,181,152,
        212,182,153,
        213,183,154,
        214,184,155,
        215,185,156,
        216,186,157,
        217,187,158,
        218,188,159,
        219,189,160,
        220,190,161,
        221,191,162,
        222,192,163,
        223,193,164,
        224,194,165,
        225,195,166,
        226,196,167,
        227,197,168,
        228,198,169,
        229,199,170,
        230,200,171,
        231,201,172,
        232,202,173,
        233,203,174,
        234,204,175,
        235,205,176,
        236,206,177,
        237,207,178,
        238,208,179,
        239,209,180,
        240,210,181,
        241,211,182,
        242,212,183,
        243,213,184,
        244,214,185,
        245,215,186,
        246,216,187,
        247,217,188,
        248,218,189,
        249,219,190,
        250,220,191,
        251,221,192,
        252,222,193,
        253,223,194,
        254,224,195,
        255,225,196,
        255,225,196,
        255,225,196,
        255,225,196,
        255,225,196,
        255,225,197,
        255,225,197,
        255,225,197,
        255,225,197,
        255,226,198,
        255,226,198,
        255,226,198,
        255,226,198,
        255,226,198,
        255,226,199,
        255,226,199,
        255,226,199,
        255,226,199,
        255,226,199,
        255,227,200,
        255,227,200,
        255,227,200,
        255,227,200,
        255,227,200,
        255,227,201,
        255,227,201,
        255,227,201,
        255,227,201,
        255,227,201,
        255,228,202,
        255,228,202,
        255,228,202,
        255,228,202,
        255,228,202,
        255,228,202,
        255,228,203,
        255,228,203,
        255,228,203,
        255,228,203,
        255,228,203,
        255,229,204,
        255,229,204,
        255,229,204,
        255,229,204,
        255,229,204,
        255,229,204,
        255,229,205,
        255,229,205,
        255,229,205,
        255,229,205,
        255,229,205,
        255,230,205,
        255,230,205,
        255,231,205,
        255,231,209,
        255,233,214,
        255,233,217,
        255,242,233,
        255,255,255
    };
    size_t i;
    for(i=0 ; i<256 ; ++i) {
        lut->r[i] = sepia[3*i];
        lut->v[i] = sepia[3*i+1];
        lut->b[i] = sepia[3*i+2];
    }
}


/* La mission des fonctions suivantes : 
 * Remplir lut->r, lut->v et lut->b en fonction de lut->param1.*/
#define min(a,b) ((a)<(b) ? (a) : (b))
#define max(a,b) ((a)>(b) ? (a) : (b))


void LUT_augmentationLuminosite(LUT *lut) {
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = min(255, lut->param1+i);
        lut->v[i] = min(255, lut->param1+i);
        lut->b[i] = min(255, lut->param1+i);
    }
}

void LUT_diminutionLuminosite(LUT *lut) {
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = max(0, (int)(i-lut->param1));
        lut->v[i] = max(0, (int)(i-lut->param1));
        lut->b[i] = max(0, (int)(i-lut->param1));
    }
}

// Change la valeur du contraste
void LUT_augmentationContraste(LUT *lut) {
    size_t i;

    for (i = 0 ; i < 256 ; i++){
        lut->r[i] = min(255, max(0, (float)lut->param1*(i - 128.) + 128.));          
        lut->v[i] = min(255, max(0, (float)lut->param1*(i - 128.) + 128.));        
        lut->b[i] = min(255, max(0, (float)lut->param1*(i - 128.) + 128.));
    }


}
void LUT_diminutionContraste(LUT *lut) {


}

/* Celles-ci ne sont pas demandées. */
void LUT_augmentationR(LUT *lut) {
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = min(255, lut->param1+i);
        lut->v[i] = i;
        lut->b[i] = i;
    }
}
void LUT_augmentationV(LUT *lut) {
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = i;
        lut->v[i] = min(255, lut->param1+i);
        lut->b[i] = i;
    }
}
void LUT_augmentationB(LUT *lut) {
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = i;
        lut->v[i] = i;
        lut->b[i] = min(255, lut->param1+i);
    }
}
void LUT_diminutionR(LUT *lut) {
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = max(0, (int)(i-lut->param1));
        lut->v[i] = i;
        lut->b[i] = i;
    }
}
void LUT_diminutionV(LUT *lut) {
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = i;
        lut->v[i] = max(0, (int)(i-lut->param1));
        lut->b[i] = i;
    }
}
void LUT_diminutionB(LUT *lut) {
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = i;
        lut->v[i] = i;
        lut->b[i] = max(0, (int)(i-lut->param1));
    }
}
void LUT_ln(LUT *lut) {
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = min(255, log(i)*(255-lut->param1)/5.0 +lut->param1);
        lut->v[i] = min(255, log(i)*(255-lut->param1)/5.0 +lut->param1);
        lut->b[i] = min(255, log(i)*(255-lut->param1)/5.0 +lut->param1);
    }
}
void LUT_exp(LUT *lut) {
    /* on appelle LUT_ln() puis on inverse les valeurs obtenues */
    LUT_ln(lut);
    
    size_t i;
    for(i=0; i<256; i++) {
        lut->r[i] = 255-lut->r[255-i];
        lut->v[i] = 255-lut->v[255-i];
        lut->b[i] = 255-lut->b[255-i];
    }
}

void LUT_gamma(LUT *lut){}
void LUT_cos(LUT *lut){}
void LUT_sin(LUT *lut){}