#include <string.h>
#include <stdlib.h>
#include "Calque.h"


void Melange_normal    (Calque *resultat, const Calque *dessous, const Calque *dessus) {
}
void Melange_addition  (Calque *resultat, const Calque *dessous, const Calque *dessus) {}
void Melange_produit   (Calque *resultat, const Calque *dessous, const Calque *dessus) {}


static void appliquerLUT(ImageRVB *img, const LUT *lut) {
    size_t y, x;
    for(y=0 ; y<img->h ; ++y) for(x=0 ; x<img->l ; ++x) {
        *ImageRVB_pixelR(img, x, y) = lut->r[*ImageRVB_pixelR(img, x, y)];
        *ImageRVB_pixelV(img, x, y) = lut->v[*ImageRVB_pixelV(img, x, y)];
        *ImageRVB_pixelB(img, x, y) = lut->b[*ImageRVB_pixelB(img, x, y)];
    }
}
void Calque_recalculer(Calque *calque) {
    ImageRVB *img = &calque->img_calculee;
    memcpy(img->rvb, calque->img_source.rvb, 3*img->l*img->h);
    LUT* lut;
    for(lut=calque->luts.premiere ; lut ; lut=lut->suivante)
        appliquerLUT(img, lut);
}
void Calque_appliquerPremiereLUT(Calque *calque) {
    LUT* lut = calque->luts.premiere;
    appliquerLUT(&calque->img_source, lut);
    calque->luts.premiere = lut->suivante;
    free(lut);
}
static Calque* Calque_nouveau(size_t l, size_t h) {
    Calque *c = malloc(sizeof(Calque));
    if(!c)
        return NULL;
    ImageRVB_allouer(&c->img_source, l, h);
    ImageRVB_allouer(&c->img_calculee, l, h);
    ListeLUTs_allouer(&c->luts);
    c->melange = Melange_normal;
    c->opacite = 1.f;
    c->en_dessous = c->au_dessus = NULL;
    ImageRVB_remplirDegradeDebile(&c->img_source);
    return c;
}
static void Calque_detruire(Calque *c) {
    ImageRVB_desallouer(&c->img_source);
    ImageRVB_desallouer(&c->img_calculee);
    ListeLUTs_desallouer(&c->luts);
    free(c);
}
void PileCalques_allouer(PileCalques *p, size_t l, size_t h) {
    ImageRVB_allouer(&p->virtuel, l, h);
    ImageRVB_allouer(&p->rendu, l, h);
    p->courant = Calque_nouveau(l, h);
}
void PileCalques_desallouerTout(PileCalques *p) {
    ImageRVB_desallouer(&p->rendu);
    ImageRVB_desallouer(&p->virtuel);
    Calque *cur, *suiv;
    for(cur=p->courant ; cur->en_dessous ; cur=cur->en_dessous)
        ;
    for( ; cur ; cur=suiv) {
        suiv = cur->au_dessus;
        Calque_detruire(cur);
    }
}
void PileCalques_recalculer(PileCalques *p) {
    Calque *cur;
    for(cur=p->courant ; cur->en_dessous ; cur=cur->en_dessous)
        ;
    /*
    Calque 
    for( ; cur ; cur=cur->au_dessus)
        cur->melange(rendu, rendu, cur);
        */
}
void PileCalques_ajouterCalqueVierge(PileCalques *p) {}
void PileCalques_supprimerCalqueCourant(PileCalques *p) {}
void PileCalques_changerCalqueCourant(PileCalques *p, int decalage) {}
