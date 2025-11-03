#ifndef H_HUD
#define H_HUD
#include "controls.h"
// engine.h: This file contains the 'main' function. Program execution begins and ends there.
//

Vec2D FromScreen(Box scr, float x, float y);

DECLARE_NODE(HUD, Renderable)
public:
	// children to draw
	TextBox* debugInfo = new TextBox(0, 0, 100, 100);
	TextBox* inputInfo = new TextBox(250, 0, 100, 100);
	TextBox* drawSurfaceInfo = new TextBox(400, 400, 100, 100);
	TextBox* perfInfo = new TextBox(0, 430, 100, 100);
	TextBox* camInfo = new TextBox(400, 0, 100, 100);
	TextBox* logo = new TextBox(0, 0, 0, 0);
	TextBox* trackInfo = new TextBox(0, 0, 0, 0);
	// funcs
	HOOK(void, OnAddedToTree, (Node* caller), (caller));
	HOOK(void, OnCreated, (), ());
	void UpdatePerformance();
END_DECLARE_NODE()

#endif