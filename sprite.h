#pragma once
#include "engine.h"

DECLARE_NODE(Sprite, Renderable)
//int width;
//int height;
SDL_Surface* surf;
SDL_Texture* texture;
std::string filename;
public:
	Sprite* CopyRegion(Box p);
	void constructTexture(SDL_Renderer* p);
	Box GetSpriteSize();
	void SamplePoints(std::vector<Vec2D>* p); // RRGGBBAA
	HOOK(void, OnStep, (double dt, Node* parent), (dt, parent));
	HOOK(void, OnRender, (RenderCtx* p), (p));
	void UpdateTextureInfo();
	void Dispose();
	static Sprite* FromDisk(std::string filename);
};

DECLARE_NODE(AnimatedSprite, Sprite)
std::map<std::string, Sprite*> animationTable;
};

