/* En-tête du programme
*****************************************
Programme :			demineur.cpp
Auteur:				Boris Zoretic et Roméo Tsarafodiana-Bugeaud
Date de création :  2016/04/09
Description :		Ce programme est un jeu de demineur
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

using namespace std;

/*Constantes globales
****************************/
//Taille de l'écran en pixels
const int LARGEUR_FENETRE = 225;
const int HAUTEUR_FENETRE = 225;

const int FACTEUR_RALENTISSEMENT_ANIMATION = 200;  //Pour rendre l'animation plus rapide, indiquer un chiffre plus petit

//Valeurs reliées au charset 'demineurCharset.png'
const int NOMBRE_BOMBES = 10;
const int NOMBRE_IMAGES_DEMINEUR_CHARSET = 1;
const int LARGEUR_IMAGE_DEMINEUR_CHARSET = 225;
const int HAUTEUR_IMAGE_DEMINEUR_CHARSET = 225;

//Variable qui incrémente le nombre de clic (1 pour 1 case)
int comptClicUnique = 0;

//Structure qui contient le nombre de fois qu'une seule case à été cliquée
struct structPositionClicUnique
{
	//0 si la case n'a jamais été cliqué, 1 sinon
	int nbFoisCliqué = 0;
};

//***************************************** Prototypes des fonctions
bool init();
bool loadMedia();
void close();
SDL_Texture* loadTexture(std::string path);
void initialiserDemineurCharset(SDL_Rect tableauRectangles[], int nombreImages, int largeurChaqueImage, int hauteurChaqueImage);
void positionClic(int &positionClicX, int &positionClicY);
void creationRandomIndex(int randIndex[NOMBRE_BOMBES], int positionBombe[NOMBRE_BOMBES][2], int &positionClicX, int &positionClicY, int grille[81][2]);
void assignationBombe(int positionBombe[NOMBRE_BOMBES][2], int grille[81][2], int randIndex[NOMBRE_BOMBES]);
void comptBombeAdjacente(int &comptBombe, int &positionClicX, int &positionClicY, int positionBombe[][2]);
void clicBombePerdu(int &positionClicX, int &positionClicY, int positionBombe[][2], int &comptVérification, SDL_Rect* &currentBombeRect);
void dessinerTableau(int &comptVérification, int &comptBombe, int positionBombe[NOMBRE_BOMBES][2], int &positionClicX, int &positionClicY, char &rejouer, SDL_Rect* &currentCase1Rect, SDL_Rect* &currentCase2Rect, SDL_Rect* &currentCase3Rect, SDL_Rect* &currentCase4Rect, SDL_Rect* &currentCase5Rect, SDL_Rect* &currentCase6Rect, SDL_Rect* &currentCase7Rect, SDL_Rect* &currentCase8Rect, SDL_Rect* &currentCaseVideRect, bool &quit, structPositionClicUnique positionClicUnique[][250]);
void victoire(char &rejouer, bool &quit, int positionBombe[NOMBRE_BOMBES][2], SDL_Rect* currentBombeRect);
void fonctionClicUnique(structPositionClicUnique positionClicUnique[250][250], int &comptClicUnique, int positionX, int positionY);

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
LTexture caseVideTexture;
LTexture bombeTexture;
LTexture case1Texture;
LTexture case2Texture;
LTexture case3Texture;
LTexture case4Texture;
LTexture case5Texture;
LTexture case6Texture;
LTexture case7Texture;
LTexture case8Texture;

//Tableaux de rectangles de chaques images
SDL_Rect grilleRect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect caseVideRect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect bombeRect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect case1Rect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect case2Rect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect case3Rect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect case4Rect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect case5Rect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect case6Rect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect case7Rect[NOMBRE_IMAGES_DEMINEUR_CHARSET];
SDL_Rect case8Rect[NOMBRE_IMAGES_DEMINEUR_CHARSET];


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
	if (!grilleTexture.loadFromFile("demineur/grille.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'grilleRect'
		initialiserDemineurCharset(grilleRect, NOMBRE_IMAGES_DEMINEUR_CHARSET, LARGEUR_IMAGE_DEMINEUR_CHARSET, HAUTEUR_IMAGE_DEMINEUR_CHARSET);
	}

	if (!caseVideTexture.loadFromFile("demineur/CaseVide.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'caseVideRect'
		initialiserDemineurCharset(caseVideRect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
	}

	if (!bombeTexture.loadFromFile("demineur/bombe.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'bombeRect'
		initialiserDemineurCharset(bombeRect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
	}

	if (!case1Texture.loadFromFile("demineur/Case1.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'case1Rect'
		initialiserDemineurCharset(case1Rect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
	}

	if (!case2Texture.loadFromFile("demineur/Case2.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'case2Rect'
		initialiserDemineurCharset(case2Rect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
	}

	if (!case3Texture.loadFromFile("demineur/Case3.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'case3Rect'
		initialiserDemineurCharset(case3Rect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
	}

	if (!case4Texture.loadFromFile("demineur/Case4.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'case4Rect'
		initialiserDemineurCharset(case4Rect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
	}

	if (!case5Texture.loadFromFile("demineur/Case5.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'case5Rect'
		initialiserDemineurCharset(case5Rect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
	}

	if (!case6Texture.loadFromFile("demineur/Case6.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'case6Rect'
		initialiserDemineurCharset(case6Rect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
	}

	if (!case7Texture.loadFromFile("demineur/Case7.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'case7Rect'
		initialiserDemineurCharset(case7Rect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
	}

	if (!case8Texture.loadFromFile("demineur/Case8.png"))
	{
		printf("Échec de chargement de l'image ! \n");
		success = false;
	}
	else
	{
		//On initialise chaque rectangle du tableau 'case8Rect'
		initialiserDemineurCharset(case8Rect, NOMBRE_IMAGES_DEMINEUR_CHARSET, 25, 25);
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
void initialiserDemineurCharset(SDL_Rect tableauRectangles[], int nombreImages, int largeurChaqueImage, int hauteurChaqueImage)
{
	for (int i = 0; i < nombreImages; i++)
	{
		tableauRectangles[i].x = largeurChaqueImage * i;
		tableauRectangles[i].y = 0;
		tableauRectangles[i].w = largeurChaqueImage;
		tableauRectangles[i].h = hauteurChaqueImage;
	}
}

//Fonction principale du programme
int main(int argc, char* args[])
{
	//variable contenant la réponse de l'utilisateur s'il veut rejouer
	char rejouer;

	do
	{
		//Déclaration des variables
		structPositionClicUnique positionClicUnique[250][250];

		//Ces variables stockent la coordonnée (x,y) des clics gauches (un à la fois)
		int positionClicX = 0;
		int positionClicY = 0;

		int comptClic = 0; //compteur pour le nombre de clic
		int comptBombe = 0; //compteur pour le nombre de bombe adjacente au clic
		int comptVérification = 0; //compteur pour vérifier si on clic sur une bombe ou non (si oui partie perdu)

		//Cet variable contient chaque position de la grille
		int grille[81][2] = { { 0, 0 }, { 0, 25 }, { 0, 50 }, { 0, 75 }, { 0, 100 }, { 0, 125 }, { 0, 150 }, { 0, 175 }, { 0, 200 },
		{ 25, 0 }, { 25, 25 }, { 25, 50 }, { 25, 75 }, { 25, 100 }, { 25, 125 }, { 25, 150 }, { 25, 175 }, { 25, 200 },
		{ 50, 0 }, { 50, 25 }, { 50, 50 }, { 50, 75 }, { 50, 100 }, { 50, 125 }, { 50, 150 }, { 50, 175 }, { 50, 200 },
		{ 75, 0 }, { 75, 25 }, { 75, 50 }, { 75, 75 }, { 75, 100 }, { 75, 125 }, { 75, 150 }, { 75, 175 }, { 75, 200 },
		{ 100, 0 }, { 100, 25 }, { 100, 50 }, { 100, 75 }, { 100, 100 }, { 100, 125 }, { 100, 150 }, { 100, 175 }, { 100, 200 },
		{ 125, 0 }, { 125, 25 }, { 125, 50 }, { 125, 75 }, { 125, 100 }, { 125, 125 }, { 125, 150 }, { 125, 175 }, { 125, 200 },
		{ 150, 0 }, { 150, 25 }, { 150, 50 }, { 150, 75 }, { 150, 100 }, { 150, 125 }, { 150, 150 }, { 150, 175 }, { 150, 200 },
		{ 175, 0 }, { 175, 25 }, { 175, 50 }, { 175, 75 }, { 175, 100 }, { 175, 125 }, { 175, 150 }, { 175, 175 }, { 175, 200 },
		{ 200, 0 }, { 200, 25 }, { 200, 50 }, { 200, 75 }, { 200, 100 }, { 200, 125 }, { 200, 150 }, { 200, 175 }, { 200, 200 } };

		//Cet variable stock les positions des bombes générer de façon randomiser
		int positionBombe[NOMBRE_BOMBES][2];

		bool quit = false;  //flag pour la boucle principale. Elle permet de déterminer si l'on doit quitter le programme

		int indexImageCharset = 0;  //Cette variable nous permet de sélectionner une image du charset de l'homme
		int compteur = 0;	  //Ce compteur est utilisé pour déterminer à quel moment changer d'image dans 'homeCharset'

		srand(time(NULL)); //fonction utiliser pour randomiser les bombes

		int randIndex[NOMBRE_BOMBES]; //variable qui stock un index randomiser

		SDL_Event e;        //Cette variable nous permet de détecter l'événement courant que le programme doit gérer (click, touche du clavier, etc.)

		//tout les rectangles contenant chaque images utilisées pour le projet
		SDL_Rect* currentGrilleRect = &grilleRect[indexImageCharset];
		SDL_Rect* currentCaseVideRect = &caseVideRect[indexImageCharset];
		SDL_Rect* currentBombeRect = &bombeRect[indexImageCharset];
		SDL_Rect* currentCase1Rect = &case1Rect[indexImageCharset];
		SDL_Rect* currentCase2Rect = &case2Rect[indexImageCharset];
		SDL_Rect* currentCase3Rect = &case3Rect[indexImageCharset];
		SDL_Rect* currentCase4Rect = &case4Rect[indexImageCharset];
		SDL_Rect* currentCase5Rect = &case5Rect[indexImageCharset];
		SDL_Rect* currentCase6Rect = &case6Rect[indexImageCharset];
		SDL_Rect* currentCase7Rect = &case7Rect[indexImageCharset];
		SDL_Rect* currentCase8Rect = &case8Rect[indexImageCharset];


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

				//Netoyyer l'écran (Clear Screen)
				SDL_RenderClear(rendererFenetre);

				//On dessine la texture de grille
				grilleTexture.render(0, 0, currentGrilleRect);

				//Mise à jour de 'rendererFenetre' (on redesine les images (contenues dans leurs textures) dans le renderer)
				SDL_RenderPresent(rendererFenetre);


				//Le programme reste ouvert tant que quit != true 
				while (!quit)
				{
					//Handle events on queue
					while (SDL_PollEvent(&e) != 0)
					{
						//Si l'utilisateur appuiie sur le bouton 'fermer' de la fenêtre
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}

						//Si l'utilisateur fait un clic de souris
						else if (e.type == SDL_MOUSEBUTTONDOWN)
						{

							positionClic(positionClicX, positionClicY);

							//Si le compteur de clic égale 0 (on génère les bombes seulement au premier clic)
							if (comptClic == 0){

								creationRandomIndex(randIndex, positionBombe, positionClicX, positionClicY, grille);
							}

							comptBombeAdjacente(comptBombe, positionClicX, positionClicY, positionBombe);

							clicBombePerdu(positionClicX, positionClicY, positionBombe, comptVérification, currentBombeRect);

							dessinerTableau(comptVérification, comptBombe, positionBombe, positionClicX, positionClicY, rejouer, currentCase1Rect, currentCase2Rect, currentCase3Rect, currentCase4Rect, currentCase5Rect, currentCase6Rect, currentCase7Rect, currentCase8Rect, currentCaseVideRect, quit, positionClicUnique);

							victoire(rejouer, quit, positionBombe, currentBombeRect);

							comptClic++; //incrémentation du compteur clic

							comptVérification = 0; //réinitialisation de comptVérification après chaque clic

						}
					}//end while (SDL_PollEvent(&e) != 0)
				}//end while (!quit)
			}

		}

		//Free resources and close SDL
		close();

	} while (toupper(rejouer) == 'O');

	return 0;
}

//fonction qui saisie la position du clic de l'utilisateur (x,y)
void positionClic(int &positionClicX, int &positionClicY)
{

	//Ces deux variables sont utiliser dans la fonction "SDL_GetMouseState(&x, &y)" pour connaitre la position du clic
	int x = 0;
	int y = 0;

	//On assigne à ces valeurs la position exact de l'image ou le clic est effectuer
	int testX = 0;
	int testY = 0;

	SDL_GetMouseState(&x, &y); //on trouve la position du clic

	//Diviser par 25 pour connaître la position exact de la case ou le clic est effectuer (retourne un entier et non un nb à virgule)
	testX = x / 25;
	testY = y / 25;

	//L'entier trouver est multiplier par 25 pour trouver la position exact de la case ou le clic est effectuer
	positionClicX = testX * 25;
	positionClicY = testY * 25;
}

//fonction qui randomize la fonction des bombes
void creationRandomIndex(int randIndex[NOMBRE_BOMBES], int positionBombe[NOMBRE_BOMBES][2], int &positionClicX, int &positionClicY, int grille[81][2])
{
	int compteur = 0;
	int compteur1 = 0;
	int indexTemp;

	//pour chaque bombe
	while (compteur < NOMBRE_BOMBES){
		if (compteur == 0){ //si cest la premiere bombe
			randIndex[compteur] = rand() % 81; //randomise un index de 0 à 81
			compteur++;
		}
		else{ //sinon si ce n'est pas le 1ere bombe

			indexTemp = rand() % 81;//randomise un index de 0 à 81

			for (int i = 0; i < compteur; i++)  //Si les deux compteur sont egaux sa veux dire que il n'y a pas dindex pareil 
				//sinon  on en recrée un nouveau jusqua se qu'il soit different
			{
				if (indexTemp != randIndex[i]){
					compteur1++;
				}
			}

			if (compteur1 == compteur){
				randIndex[compteur] = indexTemp;
				compteur++;
			}

		}
		compteur1 = 0;
	}
	//fonction qui trie les index du plus gros au plus petit
	sort(randIndex, randIndex + NOMBRE_BOMBES);


	assignationBombe(positionBombe, grille, randIndex);

	//pour chaque bombe, réassigne une position à la bombe tant qu'il y a déja une bombe dans l'index
	for (int i = 0; i < NOMBRE_BOMBES; i++){
		while (positionBombe[i][0] == positionClicX && positionBombe[i][1] == positionClicY){
			cout << "L'index à été modifier car même que position clic" << endl;
			randIndex[i] = rand() % 81;
			assignationBombe(positionBombe, grille, randIndex);
		}
	}
}

//fonction qui assigne chaque bombe à la grille
void assignationBombe(int positionBombe[NOMBRE_BOMBES][2], int grille[81][2], int randIndex[NOMBRE_BOMBES])
{
	for (int i = 0; i < NOMBRE_BOMBES; i++){
		positionBombe[i][0] = grille[randIndex[i]][0];
		positionBombe[i][1] = grille[randIndex[i]][1];
	}
}

//fonction qui compte le nombre de bombes autour du clic
void comptBombeAdjacente(int &comptBombe, int &positionClicX, int &positionClicY, int positionBombe[][2])
{
	comptBombe = 0; //réinitialisation du compteur de bombe (pour chaque clic)

	//boucle pour le nombre de bombe sur la grille
	for (int i = 0; i < NOMBRE_BOMBES; i++){
		//Les conditions suivante vérifie chaque case autour de la position du clic (8 cases). Di une bombe est trouvée on incrémente le compteur bombe de 1
		if (positionClicX + 25 == positionBombe[i][0] && positionClicY == positionBombe[i][1]){
			comptBombe++;
		}
		if (positionClicX - 25 == positionBombe[i][0] && positionClicY == positionBombe[i][1]){
			comptBombe++;
		}
		if (positionClicX == positionBombe[i][0] && positionClicY + 25 == positionBombe[i][1]){
			comptBombe++;
		}
		if (positionClicX == positionBombe[i][0] && positionClicY - 25 == positionBombe[i][1]){
			comptBombe++;
		}
		if (positionClicX + 25 == positionBombe[i][0] && positionClicY + 25 == positionBombe[i][1]){
			comptBombe++;
		}
		if (positionClicX + 25 == positionBombe[i][0] && positionClicY - 25 == positionBombe[i][1]){
			comptBombe++;
		}
		if (positionClicX - 25 == positionBombe[i][0] && positionClicY + 25 == positionBombe[i][1]){
			comptBombe++;
		}
		if (positionClicX - 25 == positionBombe[i][0] && positionClicY - 25 == positionBombe[i][1]){
			comptBombe++;
		}
	}
}

//fonction qui afiche les bombes si la position du clic correspond à la position d'une bombe
void clicBombePerdu(int &positionClicX, int &positionClicY, int positionBombe[][2], int &comptVérification, SDL_Rect* &currentBombeRect)
{
	//Si clic sur une bombe
	for (int i = 0; i < NOMBRE_BOMBES; i++){
		//compare la position du clic avec la position de chaque bombe
		if (positionClicX == positionBombe[i][0] && positionClicY == positionBombe[i][1]){

			comptVérification++; //on incrémente le compteur vérification (utiliser plus loin pour l'affichage)

			//Affichage de chaque bombes quand la partie est perdue
			for (int j = 0; j < NOMBRE_BOMBES; j++){
				bombeTexture.render(positionBombe[j][0], positionBombe[j][1], currentBombeRect);
			}
			//update les images
			SDL_RenderPresent(rendererFenetre);
		}
	}
}

//fonction qui dessine les cases
void dessinerTableau(int &comptVérification, int &comptBombe, int positionBombe[NOMBRE_BOMBES][2], int &positionClicX, int &positionClicY, char &rejouer, SDL_Rect* &currentCase1Rect, SDL_Rect* &currentCase2Rect, SDL_Rect* &currentCase3Rect, SDL_Rect* &currentCase4Rect, SDL_Rect* &currentCase5Rect, SDL_Rect* &currentCase6Rect, SDL_Rect* &currentCase7Rect, SDL_Rect* &currentCase8Rect, SDL_Rect* &currentCaseVideRect, bool &quit, structPositionClicUnique positionClicUnique[][250])
{
	//déclaration des bombes temporaires
	int compteurNouvellePosition = 1;

	int comptBombeTempYHaut = comptBombe;
	int comptBombeTempYBas = comptBombe;

	int comptBombeTempXGauche = comptBombe;
	int comptBombeTempXDroite = comptBombe;

	int nouvellePositionYHaut = 0;
	int nouvellePositionYBas = 0;
	int positionTempY = positionClicY;

	int nouvellePositionXGauche;
	int nouvellePositionXDroite;
	int positionTempX = positionClicX;

	//Si la partie n'est pas perdu (cela veut dire que comptVérification est égal à 0), on affiche les cases en conséquence du compteurBombe
	if (comptVérification == 0){
		switch (comptBombe) //switch qui affiche l'image appropriée correspondant au nombre de bombes autour
		{
		case 1: //1 bombe autour
			case1Texture.render(positionClicX, positionClicY, currentCase1Rect);
			SDL_RenderPresent(rendererFenetre);
			break;
		case 2: //2 bombes autour
			case2Texture.render(positionClicX, positionClicY, currentCase2Rect);
			SDL_RenderPresent(rendererFenetre);
			break;
		case 3: //3 bombes autour
			case3Texture.render(positionClicX, positionClicY, currentCase3Rect);
			SDL_RenderPresent(rendererFenetre);
			break;
		case 4: //4 bombes autour
			case4Texture.render(positionClicX, positionClicY, currentCase4Rect);
			SDL_RenderPresent(rendererFenetre);
			break;
		case 5: //5 bombes autour
			case5Texture.render(positionClicX, positionClicY, currentCase5Rect);
			SDL_RenderPresent(rendererFenetre);
			break;
		case 6: //6 bombes autour
			case6Texture.render(positionClicX, positionClicY, currentCase6Rect);
			SDL_RenderPresent(rendererFenetre);
			break;
		case 7: //7 bombes autour
			case7Texture.render(positionClicX, positionClicY, currentCase7Rect);
			SDL_RenderPresent(rendererFenetre);
			break;
		case 8: //8 bombes autour
			case8Texture.render(positionClicX, positionClicY, currentCase8Rect);
			SDL_RenderPresent(rendererFenetre);
			break;
		default: //si c'est une case vide
			caseVideTexture.render(positionClicX, positionClicY, currentCaseVideRect);
			SDL_RenderPresent(rendererFenetre);

			//Vérifie les cases vers le haut. Continuer jusqu'à ce qu'une case du haut ait une bombe adjacente 
			nouvellePositionYHaut = positionTempY - (25 * compteurNouvellePosition);
			comptBombeAdjacente(comptBombeTempYHaut, positionClicX, nouvellePositionYHaut, positionBombe);

			while (comptBombeTempYHaut == 0 && nouvellePositionYHaut >= 0){

				if (nouvellePositionYHaut >= 0){
					caseVideTexture.render(positionClicX, nouvellePositionYHaut, currentCaseVideRect);
					SDL_RenderPresent(rendererFenetre);
					fonctionClicUnique(positionClicUnique, comptClicUnique, positionClicX, nouvellePositionYHaut);
				}

				compteurNouvellePosition++;
				nouvellePositionYHaut = positionTempY - (25 * compteurNouvellePosition);
				comptBombeAdjacente(comptBombeTempYHaut, positionClicX, nouvellePositionYHaut, positionBombe);
			}
			if (comptBombeTempYHaut > 0 && nouvellePositionYHaut >= 0){
				dessinerTableau(comptVérification, comptBombeTempYHaut, positionBombe, positionClicX, nouvellePositionYHaut, rejouer, currentCase1Rect, currentCase2Rect, currentCase3Rect, currentCase4Rect, currentCase5Rect, currentCase6Rect, currentCase7Rect, currentCase8Rect, currentCaseVideRect, quit, positionClicUnique);
			}

			//Vérifie les cases vers le bas. Continuer jusqu'a ce qu'une case du bas à une bombe adjacente
			compteurNouvellePosition = 1;
			nouvellePositionYBas = positionTempY + (25 * compteurNouvellePosition);
			comptBombeAdjacente(comptBombeTempYBas, positionClicX, nouvellePositionYBas, positionBombe);

			while (comptBombeTempYBas == 0 && nouvellePositionYBas < 225){

				if (nouvellePositionYBas < 225){
					caseVideTexture.render(positionClicX, nouvellePositionYBas, currentCaseVideRect);
					SDL_RenderPresent(rendererFenetre);
					fonctionClicUnique(positionClicUnique, comptClicUnique, positionClicX, nouvellePositionYBas);
				}

				compteurNouvellePosition++;
				nouvellePositionYBas = positionTempY + (25 * compteurNouvellePosition);
				comptBombeAdjacente(comptBombeTempYBas, positionClicX, nouvellePositionYBas, positionBombe);
			}
			if (comptBombeTempYBas > 0 && nouvellePositionYBas < 225){
				dessinerTableau(comptVérification, comptBombeTempYBas, positionBombe, positionClicX, nouvellePositionYBas, rejouer, currentCase1Rect, currentCase2Rect, currentCase3Rect, currentCase4Rect, currentCase5Rect, currentCase6Rect, currentCase7Rect, currentCase8Rect, currentCaseVideRect, quit, positionClicUnique);
			}

			//Vérifie les cases vers la gauche. Continuer jusqu'a ce qu'une case de gauche à une bombe adjacente
			compteurNouvellePosition = 1;
			nouvellePositionXGauche = positionTempX - (25 * compteurNouvellePosition);
			comptBombeAdjacente(comptBombeTempXGauche, nouvellePositionXGauche, positionClicY, positionBombe);

			while (comptBombeTempXGauche == 0 && nouvellePositionXGauche >= 0){

				if (nouvellePositionXGauche >= 0){
					caseVideTexture.render(nouvellePositionXGauche, positionClicY, currentCaseVideRect);
					SDL_RenderPresent(rendererFenetre);
					fonctionClicUnique(positionClicUnique, comptClicUnique, nouvellePositionXGauche, positionClicY);
				}

				compteurNouvellePosition++;
				nouvellePositionXGauche = positionTempX - (25 * compteurNouvellePosition);
				comptBombeAdjacente(comptBombeTempXGauche, nouvellePositionXGauche, positionClicY, positionBombe);
			}
			if (comptBombeTempXGauche > 0 && nouvellePositionXGauche >= 0){
				dessinerTableau(comptVérification, comptBombeTempXGauche, positionBombe, nouvellePositionXGauche, positionClicY, rejouer, currentCase1Rect, currentCase2Rect, currentCase3Rect, currentCase4Rect, currentCase5Rect, currentCase6Rect, currentCase7Rect, currentCase8Rect, currentCaseVideRect, quit, positionClicUnique);
			}

			//Vérifie les cases vers la droite. Continuer jusqu'a ce qu'une case à droite à une bombe adjacente
			compteurNouvellePosition = 1;
			nouvellePositionXDroite = positionTempX + (25 * compteurNouvellePosition);
			comptBombeAdjacente(comptBombeTempXDroite, nouvellePositionXDroite, positionClicY, positionBombe);

			while (comptBombeTempXDroite == 0 && nouvellePositionXDroite < 225){

				if (nouvellePositionXDroite < 225){
					caseVideTexture.render(nouvellePositionXDroite, positionClicY, currentCaseVideRect);
					SDL_RenderPresent(rendererFenetre);
					fonctionClicUnique(positionClicUnique, comptClicUnique, nouvellePositionXDroite, positionClicY);
				}

				compteurNouvellePosition++;
				nouvellePositionXDroite = positionTempX + (25 * compteurNouvellePosition);
				comptBombeAdjacente(comptBombeTempXDroite, nouvellePositionXDroite, positionClicY, positionBombe);
			}
			if (comptBombeTempXDroite > 0 && nouvellePositionXDroite < 225){
				dessinerTableau(comptVérification, comptBombeTempXDroite, positionBombe, nouvellePositionXDroite, positionClicY, rejouer, currentCase1Rect, currentCase2Rect, currentCase3Rect, currentCase4Rect, currentCase5Rect, currentCase6Rect, currentCase7Rect, currentCase8Rect, currentCaseVideRect, quit, positionClicUnique);
			}

			break;
		}
	}
	else //sinon la case est une bombe donc afficher le message partie perdue et demander à l'utilisateur de rejouer 
	{
		cout << "Partie perdue." << endl; //ecrit dans la console partie perdue
		"Voulez vous-rejouer?";
		do
		{
			cout << "\nEntrez o pour rejouer et n pour quitter: ";
			cin >> rejouer;
		} while (toupper(rejouer) != 'N' && toupper(rejouer) != 'O');

		quit = true;
	}

	fonctionClicUnique(positionClicUnique, comptClicUnique, positionClicX, positionClicY);
}

//vérifier si toutes les cases exceptées les bombe ont été cliquées.
void victoire(char &rejouer, bool &quit, int positionBombe[NOMBRE_BOMBES][2], SDL_Rect* currentBombeRect)
{
	if (comptClicUnique == 71) //Si oui, on affiche le message partie gagnée et demander à l'utilisateur de rejouer
	{
		cout << "Partie gagnée." << endl; //ecrit dans la console pour l'instant
		"Voulez vous-rejouer?";
		for (int j = 0; j < NOMBRE_BOMBES; j++){
			bombeTexture.render(positionBombe[j][0], positionBombe[j][1], currentBombeRect);
		}
		//update les images
		SDL_RenderPresent(rendererFenetre);
		do
		{
			cout << "\nEntrez o pour rejouer et n pour quitter: ";
			cin >> rejouer;
		} while (toupper(rejouer) != 'N' && toupper(rejouer) != 'O');

		quit = true;
		comptClicUnique = 0;
	}

}

//vérifier si la case à déja été cliquée.Sinon, on incrémente de 1 la variable comptClicUnique
void fonctionClicUnique(structPositionClicUnique positionClicUnique[250][250], int &comptClicUnique, int positionX, int positionY)
{
	//si on clique pas sur une bombe
	if (positionClicUnique[positionX][positionY].nbFoisCliqué == 0)
	{
		comptClicUnique++;
		positionClicUnique[positionX][positionY].nbFoisCliqué = 1;
	}
}
