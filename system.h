#pragma once

#include "engine.h"

class System: public Node2D
{
	SDL_Surface* winSurface = NULL;
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	bool shouldFullScreen = false;
	struct Layer {
		SDL_Texture* tex;
		int index = 0;
		Node* node;
	};
	std::vector<Layer> *stack;
private:
	int RenderNode(Node * node, int depth);
	int RenderNodeQueuedSortByZIndex(std::vector<Layer>* queue, Node* node, int depth);
	int QueuedRender();
	int DeferredRender();
public:
	RenderParams GetRenderParams();
	int RunEngine();
	int InitEngine();
	int ScreenHeight;
	int ScreenWidth;
	std::string CreatewWindow(std::string uId, int x, int y, bool fullscreen);
	int ResizeWindow(std::string Uid, int x, int y, bool fullscreen);
	Box GetWindowSize(std::string uId);
};

