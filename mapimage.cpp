#include "mapimage.h"

bool MapImage::LoadMap(std::string path)
{
	GetRoot();
	
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
				(double)y * chunkWidth }, chunkWidth, chunkHeight });
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