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
	void Render(SDL_Renderer* g, Node* node);
public:
	void DeferredRender(SDL_Renderer * g);
	Renderer();
};
