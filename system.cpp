#include "system.h"
#include "engine.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "hud.h"
#include <algorithm>
#include "camera.h"

void printspaces(int s) {
	int x;
	for (x = 0;x < s;x++)
		printf("=");
}

int System::RenderNode(Node* node, int d)
{
	/*printspaces(d);
	printf("> NODE (Cls: %s) (Id: %s)\n", node->ClassName(), node->Id.c_str());
	
	RenderParams p = this->GetRenderParams();
	if (!node->isDead) {
		node->Render(&p);
	}

	for (int i = 0; i < node->children.size(); i++) {
		Node* nd = node->children[i];

		// check if node and we can assert visibility
		if (nd->Type == "NODE2D") {
			if (((Node2D*)nd)->ShouldRender()) {
				this->RenderNode(nd, d+ 1);
				continue;
			}
		}
		this->RenderNode(nd, d + 1);
	}*/

	return 0;
}

// collate draw queue of renderable items that need to be drawn!
void System::RenderNodeQueuedSortByZIndex(std::vector<Layer> *queue, Node *node, int depth) {
	printspaces(depth);
	//printf("> NODE (Cls: %s) (Id: %s)\n", node->ClassName(), node->Id.c_str());

	// get is renderable node with draw logic
	// if it is we add it to the render cue to run the render logic
	if (!node->isDead && node->InheritsFrom(Renderable::GetClassName())) {
		queue->push_back(Layer{ nullptr, index = node->index, node });
	}
	for (int i = 0; i < node->children.size(); i++) {
		this->RenderNodeQueuedSortByZIndex(queue, node->children[i], depth + 1);
	}
}

int System::QueuedRender() {
	//printf("========\nRender Start\n========\n");
	std::vector<Layer> queue = {};
	RenderNodeQueuedSortByZIndex(&queue, this, 0);
	
	// sort render queue by zindex hints
	std::sort(queue.begin(), queue.end(), [](Layer a, Layer b) {
		return a.index < b.index;
	});

	// get render param object
	RenderCtx params = this->GetRenderParams();
	
	// Get first camera
	Camera* cam = (Camera*)this->GetChild("Camera");
	
	int pixelsPerUnit = 1;
	params.M_view = cam->GetViewMatrix();
	params.M_proj = glm::translate(glm::mat4(1), { ScreenWidth * 0.5f, ScreenHeight * 0.5f, 0 }) *
		glm::scale(glm::mat4(1), { pixelsPerUnit, pixelsPerUnit, 1 }); // Y down

	// front to back of sorted render queue, call render function
	for (std::vector<Layer>::iterator it = queue.begin(); it != queue.end(); it++) {
		// get renderable node
		Renderable *r = (Renderable*)it->node;
		
		//compute if bounds are visible
		cam->GetInFustrum(r);

		// render
		r->Render(&params);
	}
	return queue.size();
}
int System::DeferredRender()
{
	//printf("========\nRender Start\n========\n");
	this->RenderNode(this, 0);

	return 0;
}

RenderCtx System::GetRenderParams()
{
	return RenderCtx{ renderer };
}

int System::RunEngine() {
	int freq = SDL_GetPerformanceFrequency();
	long int prev_ticks = SDL_GetPerformanceCounter();
	long int prev_poll_ticks = SDL_GetPerformanceCounter();
	long int prev_step_ticks = SDL_GetPerformanceCounter();
	long int prev_step_start_ticks = 0;

	double dt = 0;
	double poll_dt = 0;
	double step_dt = 0;

	bool isRunning = true;

	HUD *hud = (HUD*)GetNode("/Camera/DebugHUD");

	
	eventbuf* evbuf = new eventbuf;

	RenderCtx renderParams{ renderer };

	while (isRunning) {
		long int ticks = SDL_GetPerformanceCounter();
		dt = (ticks - prev_ticks) / (double)freq;
		poll_dt = (ticks - prev_poll_ticks) / (double)freq;
		step_dt = (ticks - prev_step_ticks) / (double)freq;
		
		((HUD*)hud)->perfInfo->SetTextF("fps: %.2f\ntick: %.2f",
			1000 / (dt * 1000),
			1000 / ((prev_step_start_ticks - step_dt) / (double)freq * 1000));
		
		// new struct every loop as copied fields is not determinable
		SDL_Event ivent;
		while (SDL_PollEvent(&ivent) != 0) {
			if (ivent.type == SDL_QUIT) {
				isRunning = false;
			}
			//evbuf->push_back(ivent);
			// execute input events
			this->DoEvent(new input_event_args{ evbuf, ivent });
		}

		// eg: if last tick delta > 1 seconds then tick
		if (step_dt > (6.0f / 1000.0f)) {
			// execute physics
			//prev_step_start_ticks = SDL_GetPerformanceCounter();
			//step_dt = (prev_step_start_ticks - prev_step_ticks) / (double)freq;

			// run step graph
			this->OnStep(step_dt, NULL);
			prev_step_ticks = SDL_GetPerformanceCounter();
		}

		// blank frame
		//SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 255, 255, 255));
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		// render back to front
		QueuedRender();
		//DeferredRender();

		SDL_RenderPresent(renderer);

		//SDL_UpdateWindowSurface(window);
		//printf("tick => %f\n", dt);

		prev_ticks = ticks;
		//SDL_Delay(16);
	}

	delete evbuf;

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 1;
}

int System::InitEngine()
{
	InitLibs();

	SDL_Rect display;
	SDL_GetDisplayBounds(0, &display);

	this->ScreenWidth = shouldFullScreen ? display.w : display.w * .8;
	this->ScreenHeight = shouldFullScreen ? display.h : display.h * .8;

	window = SDL_CreateWindow("Window",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		this->ScreenWidth, this->ScreenHeight,
		shouldFullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_SHOWN);

	if (!window) {
		return -1;
	}
	//SDL_RenderSetLogicalSize(renderer, 640, 480);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	return 1;
}

Box System::GetWindowSize(std::string uId)
{
	return Box{ Vec2D{0, 0}, (float)this->ScreenWidth, (float)this->ScreenHeight };
}
