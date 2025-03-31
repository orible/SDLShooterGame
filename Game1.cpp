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
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "engine.h"
#include "actor.h"
#include "hud.h"
#include "camera.h";
#include "surfacedrawable.h"
#include "soundlistener.h"
#include "system.h"
#include "mapimage.h"

/// <summary>
// World root, must be a Node2D as we need raycast search functions and other helpers
/// </summary>

CameraData camera;
//Screen screen;
long int uuid = 0;

System * nodeRoot = NULL;

int main(int argc, char* args[])
{
	
	nodeRoot = new System();
	nodeRoot->SetId("Root");
	nodeRoot->ScreenWidth = 1000;
	nodeRoot->ScreenHeight = 1000;

	nodeRoot->InitEngine();

	Node* cam = new Camera();
	cam->SetId("Camera");
	nodeRoot->AddChild(cam);

	Node* base = new Node();
	base->SetId("Base");
	cam->AddChild(base);

	//Node* map = new DrawableSurface();
	//map->SetId("DrawableMap");
	//base->AddChild(map);

	//MapImage* map2 = new MapImage();
	//base->AddChild(map2);
	//map2->LoadMap("/map/map_1.png");

	//SoundListener* listener = new SoundListener();
	//listener->Setup();
	//nodeRoot->AddChild(listener);

	nodeRoot->AddChild(new Actor());

	//base->GetNode("/PlayHUD");
	nodeRoot->RunEngine();

	nodeRoot->Dispose();

	return 0;
}