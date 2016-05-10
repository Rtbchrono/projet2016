struct grille
{
	//si vide, siVideOuiNon =0, sinon 1
	int siVideOuNon = 0;
};

void randomiserBlocs(int &compteurBlocs, int bloc[])
{
	compteurBlocs = 0;

	for (int i = 0; i < 100; i++)
	{
		bloc[i] = rand() & 6 + 1;
		
	}
}

void evenement(grille positionBloc[21][10], grille grilleTetris[21][10], int bloc[], int compteurBloc, SDL_Event e, bool &quit, 
			   char &recommencer, bool &verifierCollision, bool &partiePerdue, int &posX, int &posY, int &rotation)
{
	//Handle events on queue
	while (SDL_PollEvent(&e) != 0)
	{
		//Si l'utilisateur appuiie sur le bouton 'fermer' de la fenêtre
		if (e.type == SDL_QUIT)
		{
			quit = true;
			recommencer = 'n';
			partiePerdue = true;
			verifierCollision = true;

		}

		else if (e.type == SDL_KEYDOWN)
		{
			switch (e.key.keysym.sym)
			{
			case SDLK_LEFT:
				//déplacer le tetris à gauche d'une case
				
				if (posX > 0)
				{
					posX -= 1;
				}
				//manque condition pour vérifier s'il y a une collision 

				break;

			case SDLK_RIGHT:
				//déplacer le tetris à droite d'une case
				if (posX < 9)
				{
					posX += 1;
				}

				//manque condition pour vérifier s'il y a une collision 

				break;

			case SDLK_UP: //rotations
				switch (bloc[compteurBloc])
				{
				case 1:
					switch (rotation)
					{
					case 1:
						/*
						@@@@  @
						      @
						      @
						      @
						*/

						//quand le bloc est plat et qu'on veut le mettre vertical

						positionBloc[posY][posX + 1].siVideOuNon = 0;
						//reste à vérifier collision
						positionBloc[posY + 1][posX].siVideOuNon = 1;

						positionBloc[posY][posX + 2].siVideOuNon = 0;
						//idem
						positionBloc[posY + 2][posX].siVideOuNon = 1;

						positionBloc[posY][posX + 3].siVideOuNon = 0;
						//idem
						positionBloc[posY + 3][posX].siVideOuNon = 1;

						rotation = 2;
					default:
						//quand le bloc est vertical et qu'on veut le mettre droit
						positionBloc[posY+1][posX].siVideOuNon = 0;
						//reste à vérifier collision
						positionBloc[posY][posX+1].siVideOuNon = 1;

						//quand le bloc est vertical et qu'on veut le mettre droit
						positionBloc[posY + 2][posX].siVideOuNon = 0;
						//reste à vérifier collision
						positionBloc[posY][posX + 2].siVideOuNon = 1;
						
						//quand le bloc est vertical et qu'on veut le mettre droit
						positionBloc[posY + 3][posX].siVideOuNon = 0;
						//reste à vérifier collision
						positionBloc[posY][posX + 3].siVideOuNon = 1;
						rotation = 1;
						break;
					}

				case 2:
					/*          4. @
				1.	@    3. @@@    @
					@@@, ,    @ , @@
				
				2.	@ @
					@
					@
					*/

					switch (rotation)
					{
						//figure 1 vers figure 2
					case 1:
						positionBloc[posY][posX+1].siVideOuNon=1;
						positionBloc[posY - 2][posX].siVideOuNon = 1;
						//vérifier collisions
						positionBloc[posY + 1][posX + 1].siVideOuNon = 0;
						positionBloc[posY + 1][posX + 2].siVideOuNon = 0;

						rotation = 2;
						break;

						
					  //figure 2 vers figure 3
					case 2:
						positionBloc[posY][posX];


						positionBloc[posY - 1][posX].siVideOuNon = 0;
						positionBloc[posY - 2][posX].siVideOuNon = 0;
						break;

					case 3:
						break;

					default:
						break;
					}



				default:
					break;
				}
				
				//faire une rotation vers la droite du tetris
				break;
			case SDLK_DOWN:
				//accélérer la descente du tetris
				
				//besoin du timer
				


				/*default:*/

				break;

			}//fin du switch evenements
		}
	} //fin du while sdl pool event


}