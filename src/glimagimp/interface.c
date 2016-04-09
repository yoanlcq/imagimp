/***************************************************************************
 *   Copyright (C) 2005 by Venceslas BIRI                                  *
 *   biri@univ-mlv.fr                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
#include "outils.h"
#include "interface.h"

#define COMPUTE_FPS				1
#define NOMBRE_SAMPLE_FPS		100

/// ///////////////////////////////////////////////////////////////////////////
/// outil GL
/// ///////////////////////////////////////////////////////////////////////////

void __check_gl(int line, const char *filename) {
	int err = glGetError();
	if(err != GL_NO_ERROR) {
		printf("ERROR GL : erreur dans le fichier %s à la ligne %d : %s\n",filename,line,gluErrorString(err));
		exit(0);
	}
}

#define CHECK_GL __check_gl(__LINE__, __FILE__)


/// ///////////////////////////////////////////////////////////////////////////
/// variables globales
/// ///////////////////////////////////////////////////////////////////////////

/// L'ecran
unsigned int width_image = 800;
unsigned int height_image = 600;
unsigned int width_ecran = 800;
unsigned int height_ecran = 600;
unsigned char *image_src = NULL;

/// Flag pour le dessin
int flag_dessin[5];
int flag_test = 0;
int flag_fichier = 0;
int modified_tampon = 1;
unsigned int cptidletime;
unsigned int instantPrec;

/// Fonction de callback fixé par l'utilisateur
void (*fct_clic_souris_user)(int,int,int,int) = NULL;
void (*fct_clavier_user)(unsigned char,int,int) = NULL;
void (*fct_clavier_spe_user)(int,int,int) = NULL;
void (*fct_dessin_user)(void) = NULL;

/// ///////////////////////////////////////////////////////////////////////////
/// Fonctions de definition des callbacks user
/// ///////////////////////////////////////////////////////////////////////////
void fixeFonctionClavier(void (*fct)(unsigned char,int,int)) {
	fct_clavier_user = fct;
}
void fixeFonctionClavierSpecial(void (*fct)(int,int,int)) {
	fct_clavier_spe_user = fct;
}
void fixeFonctionClicSouris(void (*fct)(int button,int state,int x,int y)) {
	fct_clic_souris_user = fct;
}
void fixeFonctionDessin(void (*fct)(void)) {
	fct_dessin_user = fct;
}
void actualiseImage(unsigned char* newImage) {
	image_src = newImage;
}


/// ///////////////////////////////////////////////////////////////////////////
/// Corps des fonctions IHM
/// ///////////////////////////////////////////////////////////////////////////
void clickMouse_GLIMAGIMP(int button,int state,int x,int y) {
	if (fct_clic_souris_user != NULL) {
		(*fct_clic_souris_user)(button,state,x,y);
	}
	else {
		printf("Fonction souris non fixée\n");
	}
	glutPostRedisplay();
}

void kbdSpFunc_GLIMAGIMP(int c, int x, int y) {
	if (fct_clavier_spe_user != NULL) {
		(*fct_clavier_spe_user)(c,x,y);
	}
	else {
		printf("Fonction clavier touche spéciales non fixée\n");
	}
	glutPostRedisplay();
}

void kbdFunc_GLIMAGIMP(unsigned char c, int x, int y) {
	if (fct_clavier_user != NULL) {
		(*fct_clavier_user)(c,x,y);
	}
	else {
		printf("Fonction clavier non fixée\n");
	}
	glutPostRedisplay();
}


/// ///////////////////////////////////////////////////////////////////////////
/// Corps des fonctions de rendu GLUT
/// ///////////////////////////////////////////////////////////////////////////

/// ///////////////////////////////////////////////////////////////////////////
/// fonction de changement de dimension de la fenetre param?tres :
/// - width  : largeur (x) de la zone de visualisation 
/// - height : hauteur (y) de la zone de visualisation 
void reshapeFunc(int width,int height) {
	// ON IGNORE LE RESIZE
	glutReshapeWindow(width_ecran,height_ecran);
	// printf("Nouvelle taille %d %d\n",width_ecran,height_ecran);
	glViewport( 0, 0, (GLint)width_ecran, (GLint)height_ecran );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D(0.0,1.0,0.0,1.0);
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	modified_tampon = 1;
	glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////////
/// fonction d'exectution de process en attendant un evenement 
void idleFunc(void)
{
	cptidletime++;
	if (flag_test && COMPUTE_FPS && (cptidletime%NOMBRE_SAMPLE_FPS == 0)) {
		int instant = glutGet(GLUT_ELAPSED_TIME);
		double t=(instant-instantPrec)/1000.0;
		printf("FPS : %f\n",NOMBRE_SAMPLE_FPS/t);
		instantPrec = instant;
		glutPostRedisplay();
	}
}


/// ///////////////////////////////////////////////////////////////////////////
/// Corps des fonctions de dessin
/// ///////////////////////////////////////////////////////////////////////////
void initDisplay() {

	flag_dessin[0] = 1;
	flag_dessin[1] = 0;
	flag_dessin[2] = 0;
	flag_dessin[3] = 0;
	flag_dessin[4] = 0;

	instantPrec = glutGet(GLUT_ELAPSED_TIME);

	/// INITIALISATION DES TEXTURES ...
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);

	/// INITIALISATION DES FONCTIONS SPECIALES ...

	/// INITIALISATION CLASSIQUE OPENGL ...
	glDisable(GL_NORMALIZE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);

	CHECK_GL;
	//printf("Fin initialisation display\n");
}

void drawScene_GLIMAGIMP(void) {
	modified_tampon = 1;

	// ****************** MISE EN PLACE DES MATRICES DE DEPLACEMENT CAMERA ***********
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	/// ****************** DESSIN DE L'IMAGE ***************************************
	//printf("Ici avec %d / %d")
	glDrawPixels(width_image,height_image,GL_RGB,GL_UNSIGNED_BYTE,image_src);

	/// ****************** DESSIN CLASSIQUE ****************************************
	if (fct_dessin_user != NULL) {
		(*fct_dessin_user)();
	}
	/// ****************** SAUVEGARDE FICHIER **************************************
	if (flag_fichier) {
		printf("Sauvegarde dans le fichier");
	}

	glutSwapBuffers();
}

/// ///////////////////////////////////////////////////////////////////////////
/// Fonction d'initialisation
/// ///////////////////////////////////////////////////////////////////////////
void initGLIMAGIMP_IHM(unsigned int w_im,unsigned int h_im,unsigned char *tabRVB,
											unsigned int w_ecran,unsigned int h_ecran)
{
  ////// parametres GL /////////////////////////////////////
	cptidletime = 0;

	flag_test = 0;
	flag_fichier = 0;

	width_ecran = w_ecran;
	height_ecran = h_ecran;
	width_image = w_im;
	height_image = h_im;
	if ((width_image>width_ecran) || (height_image>height_ecran)) {
		printf("ERREUR : la taille de l'image est plus grande que la taille ecran");
		exit(1);
	}
	image_src = tabRVB;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	initDisplay();

	int argc = 0;
	char** argv = (char**) malloc(sizeof(char*));
	argv[0] = (char*) calloc(20,sizeof(char));
	sprintf(argv[0],"imagimp");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width_ecran, height_ecran);
	if (glutCreateWindow("IMAGIMP") == GL_FALSE) {
		printf("Impossible de créer la fenetre GL\n");
		exit(1);
	}
	//if (fullscreen) glutFullScreen();

	/* association de la fonction de dimensionnement */
	glutReshapeFunc(reshapeFunc);
	/* association de la fonction d'affichage */
	glutDisplayFunc(drawScene_GLIMAGIMP);
	/* association de la fonction d'événement souris */
	glutMouseFunc(clickMouse_GLIMAGIMP);
	/* association de la fonction de DRAG */
	//glutMotionFunc(motionFunc);
	/* association de la fonction d'événement du clavier */
	glutKeyboardFunc(kbdFunc_GLIMAGIMP);
	/* association de la fonction de traitement des touches*/
	/* spéciales du clavier                                */
	glutSpecialFunc(kbdSpFunc_GLIMAGIMP);
	/* fonction d'attente */
	glutIdleFunc(idleFunc);

	/* Ready to go! */
	glutMainLoop();
}

/// ///////////////////////////////////////////////////////////////////////////
/// Fonction d'initialisation
/// ///////////////////////////////////////////////////////////////////////////
void initGLIMAGIMP(unsigned int w,unsigned int h,unsigned char *tabRVB)
{
	initGLIMAGIMP_IHM(w,h,tabRVB,w,h);
}


/// ///////////////////////////////////////////////////////////////////////////
/// AUTRES OUTILS
/// ///////////////////////////////////////////////////////////////////////////

void printInfo() {
	printf("\n");
	printf("**************************************\n");
	printf("              INFORMATIONS            \n");
	printf("Taille ecran : %d/%d",width_ecran,height_ecran);
	printf("Taille image : %d/%d",width_image,height_image);
	printf("Fonction dessin ");
	if (fct_dessin_user == NULL) printf("non");
	printf(" fixée\n");
	printf("Fonction souris ");
	if (fct_clic_souris_user == NULL) printf("non");
	printf(" fixée\n");
	printf("Fonction clavier ");
	if (fct_clavier_user == NULL) printf("non");
	printf(" fixée\n");
	printf("Fonction clavier speciale ");
	if (fct_clavier_spe_user == NULL) printf("non");
	printf(" fixée\n");
	printf("**************************************\n");
}

