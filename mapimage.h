#ifndef H_MAPIMAGE_H
#define H_MAPIMAGE_H

#include "engine.h"

class MapImage: public Solid2D
{
	std::vector<Sprite> chunks;
	int chunkHeight;
	int chunkWidth;

	bool LoadMap(std::string path);
	void Render(RenderParams * g);
};


#endif // H_MAPIMAGE_H