#ifndef H_MAPIMAGE_H
#define H_MAPIMAGE_H

#include "engine.h"
#include "sprite.h"

DECLARE_NODE(MapImage, Renderable)
	//CLASSNAME(MapImage);		
	Sprite* mapImageSource;
	std::vector<Sprite> chunks;
	int chunkHeight;
	int chunkWidth;

public:
	bool LoadMap(std::string path);
	HOOK(void, OnAddedToTree, (Node* caller), (caller));
};


#endif // H_MAPIMAGE_H