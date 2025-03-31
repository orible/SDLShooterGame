#ifndef H_MAPIMAGE_H
#define H_MAPIMAGE_H

#include "engine.h"

class MapImage: public Renderable
{
	CLASSNAME(MapImage);		
	Sprite* mapImageSource;
	std::vector<Sprite> chunks;
	int chunkHeight;
	int chunkWidth;

public:
	//static bool FromFile();
	bool LoadMap(std::string path);
};


#endif // H_MAPIMAGE_H