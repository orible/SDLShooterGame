// Game1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include "engine.h"
#include "actor.h"
#include "hud.h"
#include "surfacedrawable.h"

/// <summary>
// World root, must be a Node2D as we need raycast search functions and other helpers
/// </summary>

Camera camera;
Screen screen;
long int uuid = 0;

Node2D * nodeRoot = NULL;
bool shouldFullScreen = false;

int main(int argc, char* args[])
{
	SDL_Surface* winSurface = NULL;
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	screen.width = 640;
	screen.height = 480;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return -1;
	}

	SDL_Rect display;
	SDL_GetDisplayBounds(0, &display);

	if (shouldFullScreen) {
		screen.width = display.w;
		screen.width = display.h;
	}

	window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen.width, screen.height, shouldFullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_SHOWN);

	if (!window) {
		return -1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	SDL_RenderSetLogicalSize(renderer, 640, 480);

	TTF_Init();

	nodeRoot = new Node2D_Test();
	nodeRoot->SetId("Root");

	Node* map = new DrawableSurface();
	map->SetId("DrawableMap");
	nodeRoot->AddChild(map);

	// hud goes in last as it's on top!
	Node* hud = new HUD();
	hud->SetId("PlayHUD");
	nodeRoot->AddChild(hud);
	nodeRoot->GetNode("/PlayHUD");


	long int prev_ticks = SDL_GetPerformanceCounter();
	long int prev_poll_ticks = SDL_GetPerformanceCounter();

	double dt = 0;
	double poll_dt = 0;
	int freq = SDL_GetPerformanceFrequency();
	bool isRunning = true;

	eventbuf * evbuf = new eventbuf;

	nodeRoot->AddChild(new Actor());

	while (isRunning) {
		long int ticks = SDL_GetPerformanceCounter();
		dt = (ticks - prev_ticks) / (double)freq;
		poll_dt = (ticks - prev_poll_ticks) / (double)freq;
		
		// new struct every loop as copied fields is not determinable
		SDL_Event ivent;
		while (SDL_PollEvent(&ivent) != 0) {
			if (ivent.type == SDL_QUIT) {
				isRunning = false;
			}

			//evbuf->push_back(ivent);
			// execute input events
			nodeRoot->DoEvent(new input_event_args{ evbuf, ivent });
		}
		poll_dt = prev_ticks;

		// execute physics
		nodeRoot->Step(dt, NULL);

		// blank frame
		//SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 255, 255, 255));

		SDL_RenderClear(renderer);

		// render back to front
		nodeRoot->Render(renderer);
		
		SDL_RenderPresent(renderer);
		//SDL_UpdateWindowSurface(window);

		// delay betwween last frame and now
		((HUD*)hud)->perfInfo->SetTextF("%.2f", 1000 / (dt * 1000));
		//printf("tick => %f\n", dt);

		prev_ticks = ticks;
		SDL_Delay(16);
	}

	printf("END\n");
	system("pause");

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}