#include "controls.h";

void TextBox::SetTextF(const char* fmt, ...) {
	va_list argp;
	va_start(argp, fmt);

	char* buf = new char[255];
	int ret = vsnprintf(buf, 255, fmt, argp);
	std::string text(buf, ret);
	SetText(text);
	va_end(argp);
}
void TextBox::SetText(std::string text) {
	this->text = text;
	this->dirty = true;
}
bool TextBox::IsDirty() {
	return this->dirty;
}
TextBox::TextBox(int x, int y, int width, int height) : Surface() {
	this->localPos.x = x;
	this->localPos.y = y;
}
void TextBox::Render(SDL_Renderer* g) {
	if (this->dirty) {
		if (this->tex != NULL) {
			SDL_DestroyTexture(this->tex);
			this->tex = NULL;
		}
		SDL_Color textColor = { 255, 255, 255, 0 };
		SDL_Color bgColor = { 10, 10, 10, 0 };
		SDL_Surface* surf = TTF_RenderText_Shaded_Wrapped(this->font, this->text.c_str(), textColor, bgColor, 0);
		if (surf == NULL) {
			Surface::Render(g);
			return;
		}
		fwidth = surf->w;
		fheight = surf->h;
		tex = SDL_CreateTextureFromSurface(g, surf);
		SDL_FreeSurface(surf);
		this->dirty = false;
	}
	GetGlobalPositionTransform();;
	SDL_Rect rect{ this->globalPos.x, this->globalPos.y, fwidth, fheight };
	SDL_RenderCopy(g, tex, NULL, &rect);
	Surface::Render(g);
}