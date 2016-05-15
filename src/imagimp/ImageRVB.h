#ifndef IMAGERVB_H
#define IMAGERVB_H

#include <stdbool.h>
#include <stdint.h>

struct ImageRVB {
    uint8_t *rvb;
    size_t l, h;
};
typedef struct ImageRVB ImageRVB;

typedef uint32_t Histogramme[256];

static uint8_t ImageRVB_faux_pixel[3];
#include <string.h>
static inline uint8_t *ImageRVB_pixelR(const ImageRVB *img, size_t x, size_t y) {
    if(x>=img->l || y>=img->h) {
        memset(ImageRVB_faux_pixel, 0, 3);
        return ImageRVB_faux_pixel;
    }
    return img->rvb+3*(img->l*y+x);
}
static inline uint8_t *ImageRVB_pixelV(const ImageRVB *img, size_t x, size_t y) {
    return ImageRVB_pixelR(img, x, y)+1;
}
static inline uint8_t *ImageRVB_pixelB(const ImageRVB *img, size_t x, size_t y) {
    return ImageRVB_pixelR(img, x, y)+2;
}

bool ImageRVB_allouer(ImageRVB *img, size_t largeur, size_t hauteur);
void ImageRVB_desallouer(ImageRVB *img);

/* Rouge en (0,0). 
 * Du vert est ajouté proportionnellement à x.
 * Du bleu est ajouté proportionnellement à y.
 */
void ImageRVB_remplirDegradeDebile(ImageRVB *img);

/* cote: le côté de chaque carré de l'échiquier. */
void ImageRVB_remplirEchiquier(ImageRVB *img, size_t cote, 
                               uint8_t gris_clair, uint8_t gris_sombre);

void ImageRVB_remplirRVB(ImageRVB *img, uint8_t r, uint8_t v, uint8_t b);

/* dst et src doivent bien ^etre deux images différentes dans la mémoire. */
void ImageRVB_copierSymetrieVerticale(ImageRVB * restrict dst, const ImageRVB * restrict src);

/* importer() charge l'image dynamiquement. Il faut penser à la désallouer 
 * quand on n'en a plus besoin. */
bool ImageRVB_importerPPM(ImageRVB *img, const char *nom_fichier);
bool ImageRVB_exporterPPM(const ImageRVB *img, const char *nom_fichier);

void ImageRVB_histogrammeRVB(const ImageRVB *img, Histogramme histogramme);
void ImageRVB_histogrammeR  (const ImageRVB *img, Histogramme histogramme);
void ImageRVB_histogrammeV  (const ImageRVB *img, Histogramme histogramme);
void ImageRVB_histogrammeB  (const ImageRVB *img, Histogramme histogramme);

#endif /* IMAGERVB_H */
