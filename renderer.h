#pragma once
#include <SDL.h>
#include "engine.h"

DECLARE_NODE(Renderer, Node2D)
	struct RenderLayer {
		int index;
		SDL_Texture* tex;
	};
	std::vector<RenderLayer>* stack1 = new std::vector<RenderLayer>();
	std::vector<RenderLayer>* stack2 = new std::vector<RenderLayer>();
private:
	void Render(RenderCtx* p, Node* node);
public:
	void DeferredRender(RenderCtx* p);
END_DECLARE_NODE();

