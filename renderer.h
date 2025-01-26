#pragma once
#include <SDL.h>
#include "engine.h"
class Renderer: Node2D
{
	struct RenderLayer {
		int index;
		SDL_Texture* tex;
	};

	//unsigned int stack1_offset;

	std::vector<RenderLayer>* stack1;
	std::vector<RenderLayer>* stack2;
private:
	void Render(RenderParams* p, Node* node);
public:
	void DeferredRender(RenderParams* p);
	Renderer();
};

