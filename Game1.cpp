// Game1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <minwindef.h>
#include <winnt.h>
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
#include "camera.h";
#include "surfacedrawable.h"
#include "soundlistener.h"
#include "system.h"

/// <summary>
// World root, must be a Node2D as we need raycast search functions and other helpers
/// </summary>

CameraData camera;
//Screen screen;
long int uuid = 0;

System * nodeRoot = NULL;
bool shouldFullScreen = true;

int main(int argc, char* args[])
{
	SDL_Surface* winSurface = NULL;
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	nodeRoot = new System();
	nodeRoot->SetId("Root");


	nodeRoot->ScreenWidth = 1000;
	nodeRoot->ScreenHeight = 1000;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return -1;
	}

	SDL_Rect display;
	SDL_GetDisplayBounds(0, &display);

	if (shouldFullScreen) {
		nodeRoot->ScreenWidth = display.w;
		nodeRoot->ScreenHeight = display.h;
	}

	window = SDL_CreateWindow("Window", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		nodeRoot->ScreenWidth, nodeRoot->ScreenHeight, 
		shouldFullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_SHOWN);

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

	//SDL_RenderSetLogicalSize(renderer, 640, 480);

	TTF_Init();

	Node* cam = new Camera();
	cam->SetId("Camera");
	nodeRoot->AddChild(cam);

	Node* base = new Node();
	base->SetId("Base");
	cam->AddChild(base);

	Node* map = new DrawableSurface();
	map->SetId("DrawableMap");
	base->AddChild(map);

	// hud goes in last as it's on top!
	Node* hud = new HUD();
	hud->SetId("DebugHUD");
	nodeRoot->AddChild(hud);

	SoundListener* listener = new SoundListener();
	listener->Setup();
	nodeRoot->AddChild(listener);

	//base->GetNode("/PlayHUD");


	long int prev_ticks = SDL_GetPerformanceCounter();
	long int prev_poll_ticks = SDL_GetPerformanceCounter();
	long int prev_step_ticks = SDL_GetPerformanceCounter();
	long int prev_step_start_ticks = 0;

	double dt = 0;
	double poll_dt = 0;
	double step_dt = 0;
	
	int freq = SDL_GetPerformanceFrequency();
	bool isRunning = true;

	eventbuf * evbuf = new eventbuf;

	nodeRoot->AddChild(new Actor());
	RenderParams renderParams{ renderer };
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
			nodeRoot->DoEvent(new input_event_args{ evbuf, ivent });
		}

		// eg: if last tick delta > 1 seconds then tick
		if (step_dt > (6.0f/1000.0f)) {
			// execute physics
			//prev_step_start_ticks = SDL_GetPerformanceCounter();
			//step_dt = (prev_step_start_ticks - prev_step_ticks) / (double)freq;

			nodeRoot->Step(step_dt, NULL);
			prev_step_ticks = SDL_GetPerformanceCounter();
		}

		// blank frame
		//SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 255, 255, 255));
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		// render back to front
		nodeRoot->RenderGraph(&renderParams);
		
		SDL_RenderPresent(renderer);
		//SDL_UpdateWindowSurface(window);

		//printf("tick => %f\n", dt);

		prev_ticks = ticks;
		//SDL_Delay(16);
	}

	nodeRoot->Dispose();
	//printf("END\n");
	//system("pause");

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}