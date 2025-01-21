#include "surfacedrawable.h"
#include "engine.h"
#include "hud.h"

void DrawableSurface::Add(Vec2D p)
{
	this->queue.push_back(TexPoint{ (int)p.x, (int)p.y });
}

void DrawableSurface::constructTexture(SDL_Renderer* g) {
	if (tex == NULL) {
		tex = SDL_CreateTexture(
			g, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
			this->width, this->height);
		SDL_SetRenderTarget(g, tex);
		SDL_SetRenderDrawColor(g, 0, 0, 0, 0); // Clear
		SDL_Rect rect{ 0, 0, this->width, this->width};
		SDL_RenderFillRect(g, &rect);
		SDL_SetRenderTarget(g, NULL);
	}
}

void DrawableSurface::DoEvent(input_event_args* args) {
	//computeInputVectors(args->ev);
	Surface::DoEvent(args);
}

void DrawableSurface::computeInputVectors(SDL_Event event) {
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN: {
		if (event.button.button = SDL_BUTTON_LEFT) {
			isDrawing = true;
			last.x = event.button.x;
			last.y = event.button.y;
			((HUD*)this->GetNode("/PlayHUD"))->drawSurfaceInfo->SetTextF(
				"SDL_MOUSEBUTTONDOWN: %d", totalQueue);
		}
		totalQueue++;
		//TexPoint pt{ last.x,last.y };
		//queue.push_back(pt);
	}break;
	case SDL_MOUSEBUTTONUP:
		isDrawing = false;
		break;
	case SDL_MOUSEMOTION:
		if (isDrawing) {
			int x = event.motion.x;
			int y = event.motion.y;
			TexPoint pt{ x, y };
			queue.push_back(pt);
			totalQueue++;
			((HUD*)this->GetNode("/PlayHUD"))->drawSurfaceInfo->SetTextF(
				"SDL_MOUSEMOTION: %d", totalQueue);
		}
		break;
	}
}
void DrawableSurface::DrawThickLine(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, int thickness) {
	// Calculate line length and angle
	float dx = x2 - x1;
	float dy = y2 - y1;
	float length = sqrtf(dx * dx + dy * dy);
	float angle = atan2f(dy, dx) * (180.0f / M_PI); // Convert radians to degrees

	// Create a temporary texture for the line
	SDL_Texture* lineTexture = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
		(int)length, thickness
	);

	if (lineTexture == NULL) {
		return;
	}

	// Set the texture as the render target
	SDL_SetRenderTarget(renderer, lineTexture);

	// Clear the texture (make it transparent)
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	// Draw the rectangle (the thick line)
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
	SDL_Rect rect = { 0, 0, (int)length, thickness };
	SDL_RenderFillRect(renderer, &rect);

	// Reset the render target to the default
	SDL_SetRenderTarget(renderer, nullptr);

	// Define the destination rectangle on the screen
	SDL_Rect dstRect = { (int)x1, (int)y1, (int)length, thickness };

	// Render the line texture, rotated
	SDL_RenderCopyEx(renderer, lineTexture, nullptr, &dstRect, angle, nullptr, SDL_FLIP_NONE);

	// Cleanup
	SDL_DestroyTexture(lineTexture);
}
void DrawableSurface::Render(SDL_Renderer* g) {
	constructTexture(g);
	GetGlobalPositionTransform();

	bool isDirty = queue.size() > 0;

	if (isDirty) {
		// event queue
		SDL_SetRenderDrawColor(g, 255, 255, 255, 0);
		SDL_SetRenderTarget(g, tex);
		if (queue.size() < 2) {
			SDL_RenderDrawPoint(g, queue[0].x, queue[0].y);
		}
		else {
			Vec2D lx = { tail.x, tail.y };
			for (int i = 0; i < queue.size(); i++) {
				SDL_RenderDrawLine(g, lx.x, lx.y, queue[i].x, queue[i].y);
				//SDL_RenderDrawPoint(g, queue[i].x, queue[i].y);
				lx = { (double)queue[i].x, (double)queue[i].y };
			}
			tail = lx;
			queue.clear();
		}
		SDL_SetRenderTarget(g, NULL);
	}
	
	SDL_Rect dest = { 
		this->globalPos.x, 
		this->globalPos.y, 
		this->width + this->globalPos.x, 
		this->height + this->globalPos.y
	};
	SDL_Rect src = { 0, 0, this->width, this->width};
	SDL_RenderCopy(g, tex, &src, &dest);
	Surface::Render(g);
}

DrawableSurface::DrawableSurface()
{
	this->width = 1000;
	this->height = 1000;
}
