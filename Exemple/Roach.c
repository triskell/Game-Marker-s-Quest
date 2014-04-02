/*
 ============================================================================
 Name        : Roach.c
 Author      : Thomas Abot
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "Roach.h"
#define MAX_TIME 13 //in ms ~76.92fps
#define MAX_ROACHES 3000


int main( int argc, char* args[] ) {

	/***********************
		Initialisation
	************************/
	int continuer = 1;
	int fin, debut, delta;
	int nVis;
	int deltax=10, deltay=10, btndown=0;
	char fps[10]="";
	int time_exec;

	SDL_Event event;
	TTF_Font *police = NULL;
	SDL_Surface *fond = NULL;
	SDL_Surface *ecran = init("res/floor.bmp", &fond, &police);
	SDL_Surface *sprites = LoadSprites ( "res/roach.bmp" );
	SDL_Surface *tapis = LoadImage ( "res/bathmat.bmp",X_INIT_TAPIS, Y_INIT_TAPIS );
	SDL_Surface *texte;
	SDL_Color couleur = {255,255,255};

	Roach *roaches = CreateRoaches ( ecran, sprites, MAX_ROACHES );

	SDL_Flip(ecran);


	/***********************
		Boucle de jeu
	************************/
	while(continuer){

		debut = SDL_GetTicks();

		SDL_BlitSurface(fond, NULL, ecran, NULL);

		SDL_PollEvent ( &event );

		/********************************
			Lecture des évènements
		*********************************/
		switch ( event.type ) {
			case SDL_QUIT :
				continuer = 0;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT && PointInRect( event.button.x, event.button.y,tapis->clip_rect.x,tapis->clip_rect.y,tapis->w,tapis->h)){
					//Clic sur le tapis
					btndown = 1;
					// Ecart entre clic et coin sup gauche tapis
					deltax = event.button.x - tapis->clip_rect.x;
					deltay = event.button.y - tapis->clip_rect.y;
				}
				break;
			case SDL_MOUSEMOTION :
				if ( btndown ) { // Drag and drop
					SDL_BlitSurface(fond,NULL, ecran, NULL);
					// Mise � jour position du tapis
					tapis->clip_rect.x = event.button.x - deltax;
					tapis->clip_rect.y = event.button.y - deltay;
				}
				break;
			case SDL_MOUSEBUTTONUP :
				btndown = 0;
				break;
		}

		if (event.type == SDL_KEYDOWN){
			switch ( event.key.keysym.sym ){

				case SDLK_ESCAPE :
					continuer = 0;
					break;
				default :
					break;

			}
		}


		/*************************
			Mise à jour du jeu
		**************************/

	//Mise a jour du contenu du jeu 
		nVis = MarkHiddenRoaches(roaches, MAX_ROACHES, tapis);
		if(nVis){
			MoveRoaches( roaches, MAX_ROACHES, ROACH_SPEED, ecran);
			MoveRoaches( roaches, MAX_ROACHES, ROACH_SPEED, ecran);
	//Mise a jour des graphismes 
			DrawRoaches ( roaches, MAX_ROACHES, ecran );
		}

		DrawImage ( tapis, ecran );


		//Afficher FPS et UPS
		time_exec = (delta>MAX_TIME)? delta+5 : MAX_TIME ;
		sprintf(fps, "fps:%d/ups:%d", 1000/time_exec, 2000/time_exec);
		texte = TTF_RenderText_Blended(police, fps, couleur);
		texte->clip_rect.x = 20;
		texte->clip_rect.y = 20;
		SDL_BlitSurface(texte, NULL, ecran, &(texte->clip_rect));

	//Affichage des graphismes
        SDL_Flip( ecran ); //affiche à l'écran

		fin = SDL_GetTicks();
		delta = fin-debut;

		if(delta < MAX_TIME) SDL_Delay(MAX_TIME-delta);
		else SDL_Delay(0); //dort un peu (apparemment, marche seulement avec Windows)

		}


	/**********************
		Quitter le jeu
	***********************/
	TTF_Quit();
	SDL_Quit ();

	return EXIT_SUCCESS;
}

SDL_Surface *init ( char * imgFond_filename, SDL_Surface **pFond, TTF_Font **pPolice ){

	SDL_Surface *fond;
	SDL_Surface *ecran;
	TTF_Font *police = NULL;

	srand(time(NULL)); //init random seed


	if(SDL_Init(SDL_INIT_EVERYTHING) == -1){
		fprintf(stderr, "[ERR] Erreur d'initialisation SDL : %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	fond = SDL_LoadBMP(imgFond_filename);
	if(fond == NULL) {
		fprintf(stderr, "Impossible de charger le fichier %s: %s\n", imgFond_filename, SDL_GetError());
		exit(EXIT_FAILURE);
	}

	ecran = SDL_SetVideoMode(fond->w, fond->h, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
	if(ecran == NULL) {
		fprintf(stderr, "Impossible d'initialiser l'ecran : %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_BlitSurface(fond, NULL, ecran, NULL);

	*pFond = fond;


	if(TTF_Init() == -1){
		fprintf(stderr, "[ERR] Erreur d'initialisation SDL_ttf : %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	police = TTF_OpenFont("res/The_Urban_Way.ttf", 32);
	if (police == NULL) {
		fprintf(stderr, "[ERR] Impossible de charger la police: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	*pPolice = police;

	return ecran;
}

SDL_Surface *LoadSprites ( char * sprites_filename ){

	SDL_Surface *sprites;

	sprites = SDL_LoadBMP(sprites_filename);
	if(sprites == NULL) {
		fprintf(stderr, "Impossible de charger le fichier %s: %s\n", sprites_filename, SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetColorKey ( sprites, SDL_SRCCOLORKEY, 0xFFFFFF );

	return sprites;

}

void DrawRoach ( Roach roach, SDL_Surface *ecran )
{
	SDL_Rect rect_src; // Rectangle source
	SDL_Rect rect_dest; // Rectangle destination

	rect_src.x = ( roach.dir % NB_SPRITES_P_LINE ) * ROACH_WIDTH;
	rect_src.y = ( roach.dir / NB_SPRITES_P_LINE ) * ROACH_HEIGHT;
	rect_src.w = ROACH_WIDTH;
	rect_src.h = ROACH_HEIGHT;

	rect_dest.x = roach.x;
	rect_dest.y = roach.y;

	SDL_BlitSurface(roach.sprites, &rect_src, ecran, &rect_dest);
}

int RandInt(int maxVal) {
	return rand() % maxVal;
}

/* Cr�ation d'un cafard */
Roach CreateRoach( SDL_Surface *ecran, SDL_Surface *sprites ){
	Roach roach;

	roach.sprites = sprites; // surface correspondant au sprite sheet
	roach.dir = RandInt(ROACH_ORIENTATIONS);
	roach.x = RandInt(ecran->w - ROACH_WIDTH);
	roach.y = RandInt(ecran->h - ROACH_HEIGHT);
	roach.hidden = 0;
	roach.steps = RandInt(MAX_STEPS);
	roach.angle = roach.dir * ROACH_ANGLE / 180.0 * M_PI; // radians
	roach.turnLeft = RandInt(2); // droitier: 0, gaucher: 1

	return roach;
}

int RoachInRect(int x, int y, int rectx, int recty, int rectwidth, int rectheight)
{
	if (x < rectx) return 0;
	if ((x + ROACH_WIDTH) > (rectx + rectwidth)) return 0;
	if (y < recty) return 0;
	if ((y + ROACH_HEIGHT) > (recty + rectheight)) return 0;

	return 1;
}

void TurnRoach(Roach *roach)
{
	if (roach->turnLeft) {
		roach->dir += RandInt(3) + 1; // +1 � 3
		if (roach->dir >= ROACH_ORIENTATIONS)
		roach->dir -= ROACH_ORIENTATIONS;
	} else {
		roach->dir -= RandInt(3) + 1; // -1 � 3
		if (roach->dir < 0)
			roach->dir += ROACH_ORIENTATIONS;
	}
	roach->angle = roach->dir * ROACH_ANGLE / 180.0 * M_PI; //radians
}

void MoveRoach(Roach *roaches, int nbRoaches, int index,float roachSpeed, SDL_Surface *ecran){
	Roach *roach = &roaches[index];
	int newX, newY;
	int i;

	newX = roach->x + (int)(roachSpeed * cos (roach->angle) );
	newY = roach->y - (int)(roachSpeed * sin (roach->angle) );

	// Si dans la fenetre
	if (RoachInRect(newX, newY, 0, 0, ecran->w, ecran->h)) {
	// Gestion des collisions
		for ( i = 0; i < index; i++ ) {
			if (RoachIntersectRect(newX,newY,roaches[i].x, roaches[i].y,ROACH_WIDTH, ROACH_HEIGHT)) {
			TurnRoach(roach);
			break;
			}
		}
		roach->x = newX;
		roach->y = newY;
		if (roach->steps-- <= 0) {
			TurnRoach(roach);
			roach->steps = RandInt(MAX_STEPS);
		}
	} else {
		TurnRoach(roach);
	}
}

Roach *CreateRoaches ( SDL_Surface *ecran, SDL_Surface *sprites, int nbRoach )
{
	int i;
	Roach *roaches = malloc(nbRoach*sizeof(Roach));

	for ( i = 0; i < nbRoach; i++ ) {
		roaches[i]=CreateRoach(ecran, sprites);
	}

	return roaches;
}

void DrawRoaches(Roach *roaches, int nbRoach, SDL_Surface *ecran)
{
	int i;

	for ( i = 0; i < nbRoach; i++ ) {
		if(!roaches[i].hidden)DrawRoach( roaches[i], ecran);
	}
}


void MoveRoaches ( Roach *roaches, int nbRoach, float roachSpeed,SDL_Surface *ecran)
{
	int i;

	for ( i = 0; i < nbRoach; i++ ) {
		//MoveRoach(&roaches[i], roachSpeed, ecran);//Sans gest collisions
		if(!roaches[i].hidden)MoveRoach( roaches, nbRoach, i, roachSpeed, ecran );
	}
}

int RoachIntersectRect(int x, int y, int rectx, int recty, int rectwidth, int rectheight)
{
	if (x >= (rectx + rectwidth)) return 0;
	if ((x + ROACH_WIDTH) <= rectx) return 0;
	if (y >= (recty + rectheight)) return 0;
	if ((y + ROACH_HEIGHT) <= recty) return 0;

	return 1;
}

SDL_Surface *LoadImage ( char * img_filename, int x, int y ){

	SDL_Surface *img;

	img = SDL_LoadBMP(img_filename);
	if(img == NULL) {
		fprintf(stderr, "Impossible de charger le fichier %s: %s\n", img_filename, SDL_GetError());
		exit(EXIT_FAILURE);
	}

	img->clip_rect.x = x;
	img->clip_rect.y = y;

	return img;

}

void DrawImage (SDL_Surface *img, SDL_Surface *ecran){

	SDL_BlitSurface(img, NULL, ecran, &(img->clip_rect));

}

int MarkHiddenRoaches(Roach *roaches,int nbRoaches,SDL_Surface *rect)
{
	int i;
	int nVisible = 0;

	for ( i = 0; i < nbRoaches; i++ ) {
		if (RoachInRect( roaches[i].x, roaches[i].y, rect->clip_rect.x,
			rect->clip_rect.y, rect->w, rect->h)) {
			roaches[i].hidden = 1;
		}
		else {
			roaches[i].hidden = 0;
			nVisible++;
		}
	}

	return nVisible;
}

int PointInRect(int x, int y, int rectx, int recty, int rectwidth, int rectheight) {
	if (x < rectx) return 0;
	if (x > (rectx + rectwidth)) return 0;
	if (y < recty) return 0;
	if (y > (recty + rectheight)) return 0;

	return 1;
}