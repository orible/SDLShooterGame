#include "renderer.h"


void Renderer::Render(SDL_Renderer* g, Node* node) {

	if (node->Type == "SURFACE") {
		node->Render(g);
	}

	
	SDL_Texture* tex = SDL_CreateTexture(g, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1000, 1000);
	SDL_SetRenderTarget(g, tex);
	SDL_SetRenderTarget(g, NULL);

	for (int i = 0; i < node->children.size(); i++) {
		Render(g, node->children[i]);
	}

	this->stack1->push_back(RenderLayer{ 0, tex });
}

void Renderer::DeferredRender(SDL_Renderer* g)
{
	this->stack1->clear();
	this->Render(g, this);
	for (int i = 0; i < this->stack1->size(); i++) {
		//SDL_RenderCopyEx(g, tex, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
	}

}

Renderer::Renderer()
{
	this->stack1 = new std::vector<RenderLayer>();
	this->stack2 = new std::vector<RenderLayer>();
}