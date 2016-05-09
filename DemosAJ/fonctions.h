
void randomiserBlocs(int &compteurBlocs, int bloc[])
{
	compteurBlocs = 0;

	for (int i = 0; i < 100; i++)
	{
		bloc[i] = rand() & 6 + 1;
		
	}
}
//bool verifierCollision(grille grille[], int positionBloc[])
//{
//	for (int i = 0; i < 20; i++)
//	{
//		for (int j = 0; j < 10; j++)
//		{
//
//		}
//	}
//
//}