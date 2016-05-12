#include "Calque.h"


void Melange_normal    (Calque *resultat, const Calque *dessous, const Calque *dessus) {
}
void Melange_addition  (Calque *resultat, const Calque *dessous, const Calque *dessus) {}
void Melange_produit   (Calque *resultat, const Calque *dessous, const Calque *dessus) {}


void Calque_recalculer(Calque *calque) {}
void Calque_appliquerPremiereLUT(Calque *calque) {}
void PileCalques_allouer(PileCalques *p, size_t l, size_t h) {
    ImageRVB_allouer(&p->rendu, l, h);
    ImageRVB_remplirEchiquier(&p->rendu, 16, 64, 150);
}
void PileCalques_recalculer(PileCalques *p) {}
void PileCalques_ajouterCalqueVierge(PileCalques *p) {}
void PileCalques_supprimerCalqueCourant(PileCalques *p) {}
void PileCalques_changerCalqueCourant(PileCalques *p, int decalage) {}
void PileCalques_desallouerTout(PileCalques *p) {}
