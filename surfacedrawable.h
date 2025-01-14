#ifndef DRAWABLE_SURF_H
#define DRAWABLE_SURF_H

#include "engine.h"

class DrawableSurface : public Surface {
public:
	struct TexPoint {
		int x;
		int y;
	};
	Vec2D last;
	Vec2D tail;
	bool isDrawing = false;
	SDL_Texture* tex;
	std::vector<TexPoint> queue;
	int totalQueue = 0;
	void Add(Vec2D p);
	void constructTexture(SDL_Renderer* g);
	void DoEvent(input_event_args* args);
	void computeInputVectors(SDL_Event event);
	void DrawThickLine(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, int thickness);
	void Render(SDL_Renderer* g);
};

#endif // !DRAWABLE_SURF_H
