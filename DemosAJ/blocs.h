#pragma once

using namespace std;

class blocs
{
public:
	 blocs();
	 blocs(int bloc[100]);
	~blocs();
	
	//getters
	int getType();
	int getRotation();

	//setters


	//autres m�thodes
	int rotation();
	
private:
	int _rotation;
	int _type;

};

blocs::blocs(int bloc[100])
{
}

blocs::~blocs()
{
}
