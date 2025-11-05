#include "sprite.h"
#include "system.h"
#include <SDL_image.h>

void Sprite::constructTexture(SDL_Renderer* g) {
	if (surf != NULL && texture == NULL) {
		texture = SDL_CreateTextureFromSurface(g, this->surf);
		if (texture == NULL) {
			dumpError();
			return;
		}
		// query deets
		UpdateTextureInfo();

		// dump surface out of memory
		SDL_FreeSurface(surf);
	}
}

Box Sprite::GetSpriteSize()
{
	constructTexture(((System*)this->GetRoot())->GetRenderParams().g);
	return Box{ Vec2D{0, 0}, (float)this->texwidth, (float)this->texheight };
}


/// <summary>
///	Read sprite memory and copy part of or all of it's texture
/// </summary>
/// <param name="sprite"></param>
/// <returns></returns>
Sprite* Sprite::CopyRegion(Box p)
{
	SDL_Renderer* g = ((System*)this->GetRoot())->GetRenderParams().g;
	SDL_Texture* tex = SDL_CreateTexture(g, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, p.width, p.height);

	SDL_SetRenderTarget(g, tex);
	SDL_SetRenderDrawColor(g, 100, 100, 100, 0);
	SDL_RenderClear(g);

	SDL_Rect r{ p.pos.x, p.pos.y, p.width, p.height };
	SDL_RenderCopy(g, this->texture, &r, NULL);

	SDL_SetRenderTarget(g, NULL);

	Sprite* sp = Node::New<Sprite>();
	sp->texture = tex;
	sp->UpdateTextureInfo();

	return sp;
}

void Sprite::SamplePoints(std::vector<Vec2D>* p)
{
	SDL_SetTextureBlendMode(this->texture, SDL_BLENDMODE_BLEND);

	int width, height;
	SDL_QueryTexture(this->texture, NULL, NULL, &width, &height);

	int pitch;
	void* pixels;
	SDL_LockTexture(this->texture, NULL, &pixels, &pitch);

	Uint32* upixels = (Uint32*)pixels;

	//for (int i = 0; i < width * height; i++) {}

	SDL_UnlockTexture(this->texture);
}
void Sprite::_OnStep(double dt, Node* parent) {
	//this->rot = cos(CurTime() / 1000.f);
}
void Sprite::_OnRender(RenderCtx* p) {
	// assert texture is constructed all lazy like probably boils to EQ idk, TODO: inspect the assemblr
	constructTexture(p->g);

	// compute transient fields
	SDL_Rect dstrect = { p0.x, p0.y, this->texwidth, this->texheight };
	SDL_RenderCopyEx(p->g, this->texture, NULL, &dstrect, this->rot * (100.0f / M_PI), NULL, SDL_FLIP_NONE);
}

void Sprite::UpdateTextureInfo()
{
	SDL_QueryTexture(texture, NULL, NULL, &texwidth, &texheight);
}

void Sprite::Dispose()
{
	if (this->texture != NULL) {
		SDL_DestroyTexture(this->texture);
	}
}

Sprite* Sprite::FromDisk(std::string filename)  {
	Sprite* sp = Node::New<Sprite>();
	sp->filename = filename;
	sp->surf = IMG_Load(("./assets/" + filename).c_str());
	//sp->surf = SDL_LoadBMP(("./assets/" + filename).c_str());
	return sp;
}