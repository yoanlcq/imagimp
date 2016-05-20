#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "ImageRVB.h"


bool ImageRVB_allouer(ImageRVB *img, size_t largeur, size_t hauteur) {
    img->rvb = malloc(3*largeur*hauteur);
    if(!img->rvb) {
        fprintf(stderr, "Pas assez de place pour allouer "
                        "une image RVB %ux%u.\n", 
                        (unsigned)largeur, (unsigned)hauteur);
        return false;
    }
    img->l = largeur, img->h = hauteur;
    return true;
}
void ImageRVB_desallouer(ImageRVB *img) {
    free(img->rvb);
}

/* D'après le standard http://netpbm.sourceforge.net/doc/ppm.html */
bool ImageRVB_importerPPM(ImageRVB *img, const char *nom_fichier) {

    FILE *fichier = fopen(nom_fichier, "rb");
    if(!fichier) {
        fprintf(stderr, "%s: N'a pas pu ouvrir le fichier.\n", nom_fichier);
        return false;
    }

    char car;
    unsigned variante, valeur_max;
    unsigned l, h;
    unsigned etape;
    for(etape=0 ; etape<3 ; )  {
        car = fgetc(fichier);
        if(car == EOF)
            return false;
        if(car == '#') {
            do car = fgetc(fichier); while(car != '\n' && car!=EOF);
            if(car == EOF)
                return false;
            continue;
        }
        if(!isdigit(car))
            continue;
        fseek(fichier, -1, SEEK_CUR);
        switch(etape) {
        case 0: if(fscanf(fichier, "%u", &variante)   <= 0) return false; ++etape; break;
        case 1: if(fscanf(fichier, "%u", &l)          <= 0) return false; ++etape; break;
        case 2: if(fscanf(fichier, "%u", &h)          <= 0) return false; ++etape; break;
        case 3: if(fscanf(fichier, "%u", &valeur_max) <= 0) return false; fgetc(fichier); break;
        }
    }

    if(!ImageRVB_allouer(img, l, h))
        return false;

    uint8_t *gris;
    size_t i;
    fseek(fichier, 5, SEEK_CUR); /* J'sais pas pourquoi, mais y'a besoin de faire ça. */
    switch(variante) {
    case 6: fread(img->rvb, 1, 3*img->l*img->h, fichier); break;
    case 5:
        gris = malloc(img->l*img->h);
        if(!gris) {
            fprintf(stderr, "%s: Pas de place pour allouer "
                    "%ux%u niveaux de gris.\n", nom_fichier,
                    (unsigned)img->l, (unsigned)img->h);
            ImageRVB_desallouer(img);
            return false;
        }
        fread(gris, 1, img->l*img->h, fichier);
        for(i=0 ; i<img->l*img->h ; ++i)
            img->rvb[3*i] = img->rvb[3*i+1] = img->rvb[3*i+2] = gris[i];
        free(gris);
        break;
    }
    fclose(fichier);
    return true;
}

bool ImageRVB_exporterPPM(const ImageRVB *img, const char *nom_fichier) {
    FILE *fichier = fopen(nom_fichier, "wb");
    if(!fichier) {
        fprintf(stderr, "%s: Ne peut pas ouvrir le fichier en écriture.\n", nom_fichier);
        return false;
    }
    fprintf(fichier, "P6\n%u %u\n255\n", (unsigned)img->l, (unsigned)img->h);
    fwrite(img->rvb, 1, 3*img->l*img->h, fichier);
    fclose(fichier);
    return true;
}

void ImageRVB_remplirDegradeDebile(ImageRVB *img) {
    size_t y, x;
    for(y=0 ; y<img->h ; ++y) for(x=0 ; x<img->l ; ++x)
        *ImageRVB_pixelR(img, x, y) = 255, 
        *ImageRVB_pixelV(img, x, y) = 255.f*x/(float)img->l,
        *ImageRVB_pixelB(img, x, y) = 255.f*y/(float)img->h;
}

void ImageRVB_remplirRVB(ImageRVB *img, uint8_t r, uint8_t v, uint8_t b) {
    size_t y, x;
    for(y=0 ; y<img->h ; ++y) for(x=0 ; x<img->l ; ++x)
        *ImageRVB_pixelR(img, x, y) = r, 
        *ImageRVB_pixelV(img, x, y) = v,
        *ImageRVB_pixelB(img, x, y) = b;
}

void ImageRVB_remplirEchiquier(ImageRVB *img, size_t cote, 
                               uint8_t gris_clair, uint8_t gris_sombre) {
    size_t y, x;
    bool sombre;
    for(y=0 ; y<img->h ; ++y)
        for(sombre=(y/cote)&1, x=0 ; x<img->l ; x+=cote, sombre=!sombre)
            memset(ImageRVB_pixelR(img, x, y), 
                   (sombre ? gris_sombre : gris_clair), 
                   3*(cote < img->l-x ? cote : img->l-x));
}

void ImageRVB_copier(ImageRVB * restrict dst, const ImageRVB *restrict src) {
    assert(dst != src && "Cette fonction est pour copier entre deux images"
                         " distinctes, pas d'une seule vers elle-meme.");
    size_t y, x;
    size_t minl = src->l < dst->l ? src->l : dst->l;
    size_t minh = src->h < dst->h ? src->h : dst->h;
    for(y=0 ; y<minh ; ++y) for(x=0 ; x<minl ; ++x) {
        *ImageRVB_pixelR(dst, x, y) = *ImageRVB_pixelR(src, x, y);
        *ImageRVB_pixelV(dst, x, y) = *ImageRVB_pixelV(src, x, y);
        *ImageRVB_pixelB(dst, x, y) = *ImageRVB_pixelB(src, x, y);
    }
}

void ImageRVB_copierSymetrieVerticale(ImageRVB * restrict dst, const ImageRVB * restrict src) {
    assert(dst != src && "Cette fonction est pour copier entre deux images"
                         " distinctes, pas d'une seule vers elle-meme.");
    assert(dst->l==src->l && src->l==dst->l);
    /* Ligne Haute, Ligne Basse, LarGeur */
    size_t lh, lb, lg = 3*src->l;
    for(lh=0, lb=src->h-1 ; lh<lb ; ++lh, --lb) {
        memcpy(dst->rvb + lh*lg, src->rvb + lb*lg, lg);
        memcpy(dst->rvb + lb*lg, src->rvb + lh*lg, lg);
    }
}

static void Histogramme_recalculerMinMax(Histogramme *h) {
    size_t i;
    for(h->min=UINT32_MAX, h->max=0, i=0 ; i<256 ; ++i) {
        if(h->donnees[i] > h->max)
            h->max = h->donnees[i];
        if(h->donnees[i] < h->min)
            h->min = h->donnees[i];
    }
}

void ImageRVB_histogrammeRVB(const ImageRVB *img, Histogramme *histogramme) {
    size_t x, y, somme;
    memset(histogramme, 0, sizeof(Histogramme));
    for(y=0 ; y<img->h ; ++y) {
        for(x=0 ; x<img->l ; ++x) {
            somme = *ImageRVB_pixelR(img, x, y)
                  + *ImageRVB_pixelV(img, x, y)
                  + *ImageRVB_pixelB(img, x, y);
            ++(histogramme->donnees[(uint8_t)(somme/3.f)]);
        }
    }
    Histogramme_recalculerMinMax(histogramme);
}
#define IMAGERVB_HISTOGRAMME_CANAL(C) \
void ImageRVB_histogramme##C(const ImageRVB *img, Histogramme *histogramme) { \
    size_t x, y; \
    memset(histogramme, 0, sizeof(Histogramme)); \
    for(y=0 ; y<img->h ; ++y) for(x=0 ; x<img->l ; ++x) \
        ++(histogramme->donnees[*ImageRVB_pixel##C(img, x, y)]); \
    Histogramme_recalculerMinMax(histogramme); \
}
IMAGERVB_HISTOGRAMME_CANAL(R)
IMAGERVB_HISTOGRAMME_CANAL(V)
IMAGERVB_HISTOGRAMME_CANAL(B)
#undef IMAGERVB_HISTOGRAMME_CANAL

