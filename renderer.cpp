#include "renderer.h"


void Renderer::Render(RenderParams* p, Node* node) {

	if (node->InheritsFrom(Node2D::GetTypeName())) {
		((Renderable*)node)->Render(p);
	}

	
	SDL_Texture* tex = SDL_CreateTexture(p->g, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1000, 1000);
	SDL_SetRenderTarget(p->g, tex);
	SDL_SetRenderTarget(p->g, NULL);

	for (int i = 0; i < node->children.size(); i++) {
		Render(p, node->children[i]);
	}

	this->stack1->push_back(RenderLayer{ 0, tex });
}

void Renderer::DeferredRender(RenderParams* p)
{
	this->stack1->clear();
	this->Render(p, this);
	for (int i = 0; i < this->stack1->size(); i++) {
		//SDL_RenderCopyEx(g, tex, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
	}

}

Renderer::Renderer()
{
	this->stack1 = new std::vector<RenderLayer>();
	this->stack2 = new std::vector<RenderLayer>();
}