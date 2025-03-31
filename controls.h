#ifndef CONTROLS_H
#define CONTROLS_H
#include "engine.h"

class TextBox : public Renderable {
	TTF_Font* font;
	SDL_Texture* tex;
	bool dirty = true;
	int fontSize = 24;
	std::string text;
	int fwidth, fheight;
public:
	void SetTextF(const char* fmt, ...);
	void SetText(std::string text);
	bool IsDirty();
	void SetFontSize(int size);
	Vec2D RenderedBounds();
	TextBox(int align);
	TextBox(int x, int y, int width, int height);
	void OnRender(RenderParams* p);
};

#endif