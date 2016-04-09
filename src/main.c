#include <string.h>
#include "glimagimp/interface.h"

void sur_clavier(unsigned char ascii, int x, int y) {
    printf("Touche : '%c' (souris: %d, %d)\n", ascii, x, y);
}
void sur_clavier_special(int touche, int x, int y) {
    printf("Touche spéciale : %d (souris: %d, %d)\n", touche, x, y);
    switch(touche) {
    case GLUT_KEY_F1: break;
    case GLUT_KEY_F2: break;
    case GLUT_KEY_F3: break;
    case GLUT_KEY_F4: break;
    case GLUT_KEY_F5: break; 
    case GLUT_KEY_F6: break;
    case GLUT_KEY_F7: break;
    case GLUT_KEY_F8: break;
    case GLUT_KEY_F9: break;
    case GLUT_KEY_F10: break;
    case GLUT_KEY_F11: break;
    case GLUT_KEY_F12: break;
    case GLUT_KEY_LEFT: break;
    case GLUT_KEY_UP: break;
    case GLUT_KEY_RIGHT: break;
    case GLUT_KEY_DOWN: break;
    case GLUT_KEY_PAGE_UP: break;
    case GLUT_KEY_PAGE_DOWN: break;
    case GLUT_KEY_HOME: break;
    case GLUT_KEY_END: break;
    case GLUT_KEY_INSERT: break;
    }
}
void sur_souris(int bouton, int appuye, int x, int y) {
    printf("clic: %d (%s) vers %d, %d\n", 
           bouton, appuye ? "pressé" : "laché", x, y);
}
void sur_dessin(void) {
    /* printf("dessine.\n"); */
}

int main(int argc, char *argv[]) {
    const unsigned w = 600, h = 400, ihm_w = 200;
    unsigned char rvb[3*w*h];
    memset(rvb, 0xfe, sizeof(rvb));
    fixeFonctionClavier(sur_clavier);
    fixeFonctionClavierSpecial(sur_clavier_special);
    fixeFonctionClicSouris(sur_souris);
    fixeFonctionDessin(sur_dessin);
    initGLIMAGIMP_IHM(w, h, rvb, w+ihm_w, h);
    return 0;
}
