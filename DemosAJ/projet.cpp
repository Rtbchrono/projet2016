/* En-tête du programme
*****************************************
Programme :			projet.cpp
Auteur:				Alex Rioux et Roméo Tsarafodiana-Bugeaud
Date de création :  2016/04/09
Description :		Ce programme est un jeu Tetris
*****************************************/

/*directives du préprocesseur
****************************/
#include <iostream>
#include <stdio.h>
#include <string>
#include <ctime>
#include <algorithm>
#include "../SDL2/include/SDL.h"
#include "../SDL2_image/include/SDL_image.h"


#include "fonctions.h"
using namespace std;

/*Constantes globales
****************************/
//Taille de l'écran en pixels
const int LARGEUR_FENETRE = 167;
const int HAUTEUR_FENETRE = 333;

const int FACTEUR_RALENTISSEMENT_ANIMATION = 200;  //Pour rendre l'animation plus rapide, indiquer un chiffre plus petit

//Valeurs reliées au charset 'demineurCharset.png'
const int nombre_IMAGES_TETRIS_CHARSET = 1;
const int LARGEUR_IMAGE_DEMINEUR_CHARSET = 225;
const int HAUTEUR_IMAGE_DEMINEUR_CHARSET = 225;

//***************************************** Prototypes des fonctions
bool init();
bool loadMedia();
void close();
void initialiserTetrisCharset(SDL_Rect tableauRectangles[], int nombreImages, int largeurChaqueImage, int hauteurChaqueImage);
SDL_Texture* loadTexture(std::string path);


//********************************* Variables globales (partie 1)

//La fenêtre dans laquelle le programme sera exécuté
SDL_Window* fenetreProgramme = NULL;

//Le 'renderer' associé à la fenêtre du programme. Le 'renderer' est l'endroit dans lequelle on placera des textures
SDL_Renderer* rendererFenetre = NULL;




//***************************************** Classe qui nous permet de manipuler les textures (Ne pas éditer)

////Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture();

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(rendererFenetre, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopy(rendererFenetre, mTexture, clip, &renderQuad);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

//Les variables de type LTexture de chaque image
LTexture grilleTexture;
LTexture bloc1Texture;
LTexture bloc2Texture;
LTexture bloc3Texture;
LTexture bloc4Texture;
LTexture bloc5Texture;
LTexture bloc6Texture;
LTexture bloc7Texture;


//Tableaux de rectangles de chaques images
SDL_Rect grilleRect[nombre_IMAGES_TETRIS_CHARSET];
SDL_Rect bloc1Rect[nombre_IMAGES_TETRIS_CHARSET];
SDL_Rect bloc2Rect[nombre_IMAGES_TETRIS_CHARSET];
SDL_Rect bloc3Rect[nombre_IMAGES_TETRIS_CHARSET];
SDL_Rect bloc4Rect[nombre_IMAGES_TETRIS_CHARSET];
SDL_Rect bloc5Rect[nombre_IMAGES_TETRIS_CHARSET];
SDL_Rect bloc6Rect[nombre_IMAGES_TETRIS_CHARSET];
SDL_Rect bloc7Rect[nombre_IMAGES_TETRIS_CHARSET];



//********************************* Fonctions du programme

//Cette fonction initialise SDL et créé la fenêtre du programme
bool init()
{
	//Cette variable indique si l'initialisation a été effectuée avec succès
	bool success = true;

	//Initialisation de SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Erreur lors de l'initialisation de SDL! - SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Création de la fenêtre du programme
		fenetreProgramme = SDL_CreateWindow("Titre fenetre", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, LARGEUR_FENETRE, HAUTEUR_FENETRE, SDL_WINDOW_SHOWN);

		if (fenetreProgramme == NULL)
		{
			printf("Échec de création de la fenêtre ! -  SDL_Error %s\n", SDL_GetError());
			success = false;
		}
		else 
		{
			//Creation du 'renderer' en l'associant à la fenêtre du programme
			rendererFenetre = SDL_CreateRenderer(fenetreProgramme, -1, SDL_RENDERER_ACCELERATED);
			if (rendererFenetre == NULL)
			{
				printf("Échec de création du renderer ! - SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(rendererFenetre, 0xFF, 0xAA, 0xAA, 0xAA);

				//Les commandes suivante permettent d'activer le chargement d'images dont le format est PNG
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("Échec d'initialisation de SDL_image ! -  SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

//Cette fonction est utilisée pour charger les images du programme dans leurs textures correspondantes
bool loadMedia()
{
	//Cette variable indique si le chargément de l'élément a été effectué avec succès
	bool success = true;

	//Load sprite sheet texture
	if (!grilleTexture.loadFromFile("tetris/grille.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'grilleRect'
		initialiserTetrisCharset(grilleRect, nombre_IMAGES_TETRIS_CHARSET, LARGEUR_FENETRE,HAUTEUR_FENETRE);
	}

	if (!bloc1Texture.loadFromFile("tetris/bloc1r1.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		initialiserTetrisCharset(bloc1Rect, nombre_IMAGES_TETRIS_CHARSET, 80, 82);
	}

	if (!bloc5Texture.loadFromFile("tetris/bloc4.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		initialiserTetrisCharset(bloc5Rect, nombre_IMAGES_TETRIS_CHARSET, 80,82);
	}

	if (!bloc6Texture.loadFromFile("tetris/bloc5r1.png"))
	{
		printf("Échec de chargement de l'image ! \n");
	}
	else
	{
		initialiserTetrisCharset(bloc6Rect, nombre_IMAGES_TETRIS_CHARSET, 80, 82);
	}
	
	if (!bloc7Texture.loadFromFile("tetris/bloc6r1.png"))
	{
		printf("Échec du chargement de l'image ! \n");
	}
	else
	{
		initialiserTetrisCharset(bloc7Rect, nombre_IMAGES_TETRIS_CHARSET, 80, 82);
	}

	return success;
}

//Cette fonction effectue une fermeture appropriée du programme, de l'environnement SDL et libère la mémoire des différents éléments SDL créés
void close()
{

	//On détruit les LTextures créées dans le programme
	grilleTexture.free();

	//Destroy window
	SDL_DestroyRenderer(rendererFenetre);
	SDL_DestroyWindow(fenetreProgramme);
	fenetreProgramme = NULL;
	rendererFenetre = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

//Cette fonction est responsable de charger dans une texture l'image indiqueé en paramètre
SDL_Texture* loadTexture(std::string path)
{
	//La texture finale
	SDL_Texture* newTexture = NULL;

	//Chargement de l'image dans la texture
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Échec de chargement de l'image dans la texture ! -  SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(rendererFenetre, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
}

//Initialisation des rectangles qui vont s'associer aux différentes images du charset
void initialiserTetrisCharset(SDL_Rect tableauRectangles[], int nombreImages, int largeurChaqueImage, int hauteurChaqueImage)
{
	for (int i = 0; i < nombreImages; i++)
	{
	/*	tableauRectangles[i].x = largeurChaqueImage * i*/;
		tableauRectangles[i].y = 0;
		tableauRectangles[i].w = largeurChaqueImage;
		tableauRectangles[i].h = hauteurChaqueImage;
	}
}

//Fonction principale du programme
int main(int argc, char* args[])
{
	
	//variables
	bool quit = false;
	char recommencer = 'O';
	int compteurBlocs;
	int bloc[100];
	grille Grille[20][10];
	int positionBloc[20][10];
	int indexImageCharset = 0;  //Cette variable nous permet de sélectionner une image du charset de l'homme
	SDL_Event e;  //Cette variable nous permet de détecter l'événement courant que le programme doit gérer (click, touche du clavier, etc.)
	
	//tout les rectangles utiliser pour le projet
	SDL_Rect* currentGrilleRect = &grilleRect[indexImageCharset];
	SDL_Rect* currentBloc1Rect = &bloc1Rect[indexImageCharset];

	//On définit la position initial de l'image (grille)
	int positionGrilleX = 0;
	int positionGrilleY = 0;

	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
		system("pause");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
			system("pause");
		}
		else
		{
			printf("\nProgramme OK!\n\n");
			while (quit == false)
			{
				while (toupper(recommencer) == 'O')
				{
					bool partiePerdue = false;
					bool verifierCollision = false;
					grille positionBloc[21][10];
					grille grilleTetris[21][10];

					grilleTexture.render(positionGrilleX, positionGrilleY, grilleRect);
					//initialise la derniere ligne (jeu 
					for (size_t i = 0; i < 10; i++)
					{
						grilleTetris[21][i].siVideOuNon = 1;
						positionBloc[21][i].siVideOuNon = 1;
					}

					randomiserBlocs(compteurBlocs, bloc);
					int y = 0;
					int x = 4;
					int posY;
					int posX;
					int rotation = 0;
				
					//si rotation = 0 : le bloc est droit, si rotation = 1: le bloc est à droite, si rotation = 2: bloc bas, si = 3: bloc gauche
					//si = 4: bloc est en haut
					//grilleTexture.render(0, 0, grilleRect);
					while (partiePerdue == false)
					{
						int nombreDeBlocs = 0;

						rotation = 0;

						//switch qui va prendre le bon bloc
						switch (bloc[compteurBlocs])
						{

						case 1:
							posX = 4;
							posY = 0;

							//evenement(e,quit,recommencer,verifierCollision,partiePerdue,posX,posY);
							//Update screen
							SDL_RenderPresent(rendererFenetre);

							while (verifierCollision == false)
							{	
								//si le bloc est plat
								positionBloc[posY][posX].siVideOuNon = 1;
								positionBloc[posY][posX + 1].siVideOuNon = 1;
								positionBloc[posY][posX + 2].siVideOuNon = 1;
								positionBloc[posY][posX + 3].siVideOuNon = 1;


								//si collision
								if (positionBloc[posY][posX].siVideOuNon + Grille[posY][posX].siVideOuNon == 2 ||
									positionBloc[posY][posX + 1].siVideOuNon + Grille[posY][posX + 1].siVideOuNon == 2 ||
									positionBloc[posY][posX + 2].siVideOuNon + Grille[posY][posX + 2].siVideOuNon == 2 ||
									positionBloc[posY][posX + 3].siVideOuNon + Grille[posY][posX + 2].siVideOuNon == 2)
								{
									verifierCollision = true;
									bloc1Texture.render(posX * 17, ((posY - 1) * 17));
									break;
								}

								//on descend de 1 et on remet la valeur de la case à vide
								else
								{
									positionBloc[posY][posX].siVideOuNon = 0;
									positionBloc[posY][posX + 1].siVideOuNon = 0;
									positionBloc[posY][posX + 2].siVideOuNon = 0;
									positionBloc[posY][posX + 3].siVideOuNon = 0;
								}

								//evenement(positionBloc,grilleTetris,bloc, compteurBlocs ,e, quit, recommencer, verifierCollision, partiePerdue, posX, posY, rotation);
								posY += 1;

								//Netoyyer l'écran (Clear Screen)
								SDL_RenderClear(rendererFenetre);
								grilleTexture.render(0, 0, grilleRect);
								bloc1Texture.render(posX * 17, posY * 17, bloc1Rect);
								//Update screen
								SDL_RenderPresent(rendererFenetre);
								compteurBlocs++;
							}


							//afficher bloc 1 a la position 7,25 de la grille
							//initialiser position bloc en fonction du bloc 1
							break;
						case 2:
							posX = 0;
							posY = 4;
							bloc2Texture.render(posX, posY, bloc2Rect);
							//afficher bloc 2 a la position 7,25 de la grille
							//initialiser position bloc en fonction du bloc 2
						case 3:
							//afficher bloc 3 a la position 7,25 de la grille
							//initialiser position bloc en fonction du bloc 3
							break;
						case 4:
							//afficher bloc 4 a la position 7,25 de la grille
							//initialiser position bloc en fonction du bloc 4
							break;
						case 5:
							//afficher bloc 5 a la position 7,25 de la grille
							//initialiser position bloc en fonction du bloc 5
							break;
						case 6:
							//afficher bloc 6 a la position 7,25 de la grille
							//initialiser position bloc en fonction du bloc 6
							break;
						default:
							//afficher bloc 7 a la position 7,25 de la grille
							//initialiser position bloc en fonction du bloc 7
							break;
						} //fin switch


					} //fin du while partiePerdue==false

					cout << "Partie Perdue. Voulez vous recommencer?" << endl;
					do
					{
						cout << "Entrer o pour oui, n pour non: ";
						cin >> recommencer;
					} while (toupper(recommencer) != 'O' && toupper(recommencer) != 'N');

				}//fin while
			}
			
			
		}//fin de l'accolade du else contenant le code
	}//fin du else (si le jeu s'est initialisé)
	
	return 0;
}//fin du main