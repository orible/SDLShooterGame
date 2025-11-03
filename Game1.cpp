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
	nodeRoot = Node::New<System>();
	nodeRoot->SetId("Root");
	nodeRoot->ScreenWidth = 1000;
	nodeRoot->ScreenHeight = 1000;
	nodeRoot->InitEngine();
	
	Node* cam = Node::New<Camera>();
	cam->SetId("Camera");
	nodeRoot->AddChild(cam);

	// hud goes in last as it's on top!
	Node* hud = Node::New<HUD>();
	hud->SetId("DebugHUD");
	cam->AddChild(hud);

	//Node* base = new Node();
	//base->SetId("Base");
	//cam->AddChild(base);

	//Actor* a = Node::New<Actor>();
	//a->SetLocalPos({ 100, 100 });
	//nodeRoot->AddChild(a);

	nodeRoot->AddChild(Node::New<MapImage>());

	nodeRoot->AddChild(Node::New<Actor>());
	nodeRoot->RunEngine();
	nodeRoot->Dispose();
	
	//nodeRoot->AddChild(Sprite::FromDisk("./player.bmp"));
	//nodeRoot->AddChild(Sprite::FromDisk("./player.bmp"));

	delete nodeRoot;

	return 0;
}