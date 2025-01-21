#ifndef CONTROLS_H
#define CONTROLS_H
#include "engine.h"

class TextBox : public Surface {
	TTF_Font* font = TTF_OpenFont("./assets/fonts/Broken Console Bold.ttf", 24);
	SDL_Texture* tex;
	bool dirty = true;
	std::string text;
	int fwidth, fheight;
public:
	void SetTextF(const char* fmt, ...);
	void SetText(std::string text);
	bool IsDirty();
	Vec2D RenderedBounds();
	TextBox(int align);
	TextBox(int x, int y, int width, int height);
	void Render(SDL_Renderer* g);
};

#endif