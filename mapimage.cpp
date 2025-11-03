#include "mapimage.h"

void MapImage::_OnAddedToTree(Node*caller) {
	this->LoadMap("./assets/map_1.png");
}

bool MapImage::LoadMap(std::string path)
{
	
	chunkWidth = 600;
	chunkHeight = 600;


	// load entire png into ram
	mapImageSource = Sprite::FromDisk(path);

	this->AddChild(mapImageSource);

	Box box = mapImageSource->GetSpriteSize();
	
	int numWidth = box.width / chunkWidth;
	int numHeight = box.height / chunkWidth;

	int totalChunks = numWidth * numHeight;

	for (int y = 0; y < numHeight; y++) {
		for (int x = 0; x < numWidth; x++) {
			Sprite * region = mapImageSource->CopyRegion(Box{ Vec2D{ 
				(double)x * chunkWidth, 
				(double)y * chunkWidth }, (float)chunkWidth, (float)chunkHeight });
			if (region == NULL) {
				break;
			}
			region->SetLocalPos(Vec2D{ (double)x * chunkWidth, (double)y * chunkHeight });
			this->AddChild(region);
		}
	}
	
	//mapImageSource->RemoveSelf();
	//delete mapImageSource;

	return false;
}