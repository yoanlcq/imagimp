#include <string.h>
#include <stdlib.h>
#include "Calque.h"


#define FOR_EACH_PIXEL() \
            size_t y, x; \
            for(y=0 ; y<resultat->img_calculee.h ; ++y) \
                for(x=0 ; x<resultat->img_calculee.l ; ++x)
#define OUT(C) (*ImageRVB_pixel##C(&resultat->img_calculee, x, y))
#define SRC(C) (*ImageRVB_pixel##C(&dessus->img_calculee, x, y))
#define DST(C) (*ImageRVB_pixel##C(&dessous->img_calculee, x, y))
#define OUT_A (resultat->opacite)
#define SRC_A (dessus->opacite)
#define DST_A (dessous->opacite)
#define MIN(a,b) ((a)<(b) ? (a) : (b))
void Melange_normal    (Calque *resultat, const Calque *dessous, const Calque *dessus) {
    /* Voir https://en.wikipedia.org/wiki/Alpha_compositing#Alpha_blending */
    OUT_A = SRC_A + DST_A*(1.f-SRC_A);
    FOR_EACH_PIXEL() {
        OUT(R) = (SRC(R)*SRC_A + DST(R)*DST_A*(1.f-SRC_A))/OUT_A;
        OUT(V) = (SRC(V)*SRC_A + DST(V)*DST_A*(1.f-SRC_A))/OUT_A;
        OUT(B) = (SRC(B)*SRC_A + DST(B)*DST_A*(1.f-SRC_A))/OUT_A;
    }
}
void Melange_addition  (Calque *resultat, const Calque *dessous, const Calque *dessus) {
    OUT_A = SRC_A + DST_A*(1.f-SRC_A);
    FOR_EACH_PIXEL() {
        OUT(R) = MIN(255, SRC(R)*SRC_A + DST(R));
        OUT(V) = MIN(255, SRC(V)*SRC_A + DST(V));
        OUT(B) = MIN(255, SRC(B)*SRC_A + DST(B));
    }
}
void Melange_multiplication   (Calque *resultat, const Calque *dessous, const Calque *dessus) {
    OUT_A = SRC_A + DST_A*(1.f-SRC_A);
    FOR_EACH_PIXEL() {
        OUT(R) = DST(R) + SRC_A*(255.f*(SRC(R)/255.f)*(DST(R)/255.f) - DST(R));
        OUT(V) = DST(V) + SRC_A*(255.f*(SRC(V)/255.f)*(DST(V)/255.f) - DST(V));
        OUT(B) = DST(B) + SRC_A*(255.f*(SRC(B)/255.f)*(DST(B)/255.f) - DST(B));
        /* Ca, c'est la formule de l'énoncé, mais en fait ça ressemble
         * plus au mode "normal" qu'au mode "multiplier".
        OUT(R) = SRC_A*SRC(R) + (1.f-SRC_A)*DST(R);
        OUT(V) = SRC_A*SRC(V) + (1.f-SRC_A)*DST(V);
        OUT(B) = SRC_A*SRC(B) + (1.f-SRC_A)*DST(B);
        */
    }
}
#undef FOR_EACH_PIXEL
#undef OUT
#undef SRC
#undef DST
#undef OUT_A
#undef SRC_A
#undef DST_A
#undef MIN

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
    ImageRVB_copier(img, &calque->img_source);
    LUT* lut;
    for(lut=calque->luts.premiere ; lut ; lut=lut->suivante)
        appliquerLUT(img, lut);
}

void Calque_appliquerPremiereLUT(Calque *calque) {
    appliquerLUT(&calque->img_source, calque->luts.premiere);
    ListeLUTs_retirerPremiere(&calque->luts);
}
static Calque* Calque_nouveau(size_t l, size_t h) {
    Calque *c = malloc(sizeof(Calque));
    if(!c) return NULL;
    if(!ImageRVB_allouer(&c->img_source, l, h)) return NULL;
    if(!ImageRVB_allouer(&c->img_calculee, l, h)) return NULL;
    if(!ListeLUTs_allouer(&c->luts)) return NULL;
    c->melange = Melange_normal;
    c->opacite = 1.f;
    c->en_dessous = c->au_dessus = NULL;
    ImageRVB_remplirRVB(&c->img_source, 255, 255, 255);
    Calque_recalculer(c);
    return c;
}
static void Calque_detruire(Calque *c) {
    ImageRVB_desallouer(&c->img_source);
    ImageRVB_desallouer(&c->img_calculee);
    ListeLUTs_desallouer(&c->luts);
    free(c);
}
bool PileCalques_allouer(PileCalques *p, size_t l, size_t h) {
    if(!ImageRVB_allouer(&p->virtuel, l, h)) return false;
    if(!ImageRVB_allouer(&p->rendu, l, h)) return false;
    if(!ImageRVB_allouer(&p->rendu_gl, l, h)) return false;
    if(!ImageRVB_allouer(&p->rendu_vuesource_gl, l, h)) return false;
    p->courant = Calque_nouveau(l, h);
    if(!p->courant) return false;
    return true;
}
void PileCalques_desallouerTout(PileCalques *p) {
    ImageRVB_desallouer(&p->rendu);
    ImageRVB_desallouer(&p->rendu_gl);
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
    Calque rendu;
    ImageRVB_copier(&p->rendu, &p->virtuel);
    rendu.img_calculee.rvb = p->rendu.rvb;
    rendu.img_calculee.l   = p->rendu.l;
    rendu.img_calculee.h   = p->rendu.h;
    rendu.opacite = 1.f;
    Calque *cur;
    for(cur=p->courant ; cur->en_dessous ; cur=cur->en_dessous)
        ;
    for( ; cur ; cur=cur->au_dessus)
        cur->melange(&rendu, &rendu, cur);
    ImageRVB_copierSymetrieVerticale(&p->rendu_gl, &p->rendu);
    ImageRVB_histogrammeRVB(&p->rendu, &p->histogramme_rendu_rvb);
    ImageRVB_histogrammeR  (&p->rendu, &p->histogramme_rendu_r);
    ImageRVB_histogrammeV  (&p->rendu, &p->histogramme_rendu_v);
    ImageRVB_histogrammeB  (&p->rendu, &p->histogramme_rendu_b);
}
bool PileCalques_ajouterCalqueVierge(PileCalques *p) {
    Calque *ancien = p->courant->au_dessus;
    Calque *nouveau = Calque_nouveau(p->rendu.l, p->rendu.h);
    if(!nouveau) return false;
    p->courant->au_dessus = nouveau;
    nouveau->en_dessous = p->courant;
    nouveau->au_dessus = ancien;
    if(ancien) ancien->en_dessous = nouveau;
    p->courant = nouveau;
    return true;
}
bool PileCalques_supprimerCalqueCourant(PileCalques *p) {
    Calque *dessus = p->courant->au_dessus;
    Calque *dessous = p->courant->en_dessous;
    if(!dessus && !dessous)
        return false;
    Calque_detruire(p->courant);
    if(dessous) dessous->au_dessus = dessus;
    if(dessus)  dessus->en_dessous = dessous;
    p->courant = dessous;
    return true;
}
