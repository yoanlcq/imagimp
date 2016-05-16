#include <stddef.h>
#include <stdint.h>

#include "LUT.h"
#include "ImageRVB.h"

typedef struct Calque Calque;
/* Ces fonctions prennent des Calques en paramètres et non des ImageRVB car
 * il nous faut l'information d'opacité du calque. */
/* Lecture intéressante : https://docs.gimp.org/en/gimp-concepts-layer-modes.html */
typedef void (*FonctionMelange)(Calque *resultat, const Calque *dessous, const Calque *dessus);
void Melange_normal    (Calque *resultat, const Calque *dessous, const Calque *dessus);
void Melange_addition  (Calque *resultat, const Calque *dessous, const Calque *dessus);
void Melange_produit   (Calque *resultat, const Calque *dessous, const Calque *dessus);

struct Calque {
    ImageRVB img_source, img_calculee;
    ListeLUTs luts;
    FonctionMelange melange;
    float opacite;
    Calque *en_dessous, *au_dessus;
};

typedef struct {
    Calque *courant; /* C'est la aussi la liste doublement chaînée de 
                        tous les calques. */
    /* rendu_gl résoud deux problèmes:
     * 1) glimagimp affiche les images à l'envers, verticalement, on doit donc
     *    envoyer à actualiseImage() une copie retournée de l'image;
     * 2) Le pointeur passé à actualiseImage() doit etre valide suffisament
     *    longtemps pour que le driver OpenGL aie le temps de le récupérer.
     *    En gros, ça crashe souvent si on passe un tableau dont la durée de
     *    vie est courte à actualiseImage().
     * J'ai essayé de contourner le problème avec gluOtho2D() et glPixelZoom(),
     * sans succès, donc j'ai opté pour cette solution crade.
     */
    ImageRVB rendu;
    ImageRVB rendu_gl;
    ImageRVB rendu_vuesource_gl;
    ImageRVB virtuel;
    Histogramme histogramme_rendu_rvb;
    Histogramme histogramme_rendu_r;
    Histogramme histogramme_rendu_v;
    Histogramme histogramme_rendu_b;
    Histogramme histogramme_vuesource_rvb;
    Histogramme histogramme_vuesource_r;
    Histogramme histogramme_vuesource_v;
    Histogramme histogramme_vuesource_b;
} PileCalques;

void Calque_recalculer(Calque *calque);
void Calque_appliquerPremiereLUT(Calque *calque);
void PileCalques_allouer(PileCalques *p, size_t l, size_t h);
void PileCalques_recalculer(PileCalques *p);
void PileCalques_ajouterCalqueVierge(PileCalques *p);
void PileCalques_supprimerCalqueCourant(PileCalques *p);
void PileCalques_desallouerTout(PileCalques *p);
