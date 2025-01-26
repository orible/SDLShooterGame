#include "mapimage.h"

bool MapImage::LoadMap(std::string path)
{
	chunkWidth = 600;
	chunkHeight = 600;

	// load entire png into ram
	Sprite* sp = Sprite::FromDisk(path);

	Box box = sp->GetSpriteSize();
	
	int numWidth = box.width / chunkWidth;
	int numHeight = box.height / chunkWidth;

	int totalChunks = numWidth * numHeight;

	this->

	for (int i = 0; i < totalChunks; i++) {
	}

	return false;
}

void MapImage::Render(RenderParams* g)
{
	Solid2D::Render(g);
}
