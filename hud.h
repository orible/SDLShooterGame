#ifndef H_HUD
#define H_HUD
#include "controls.h"
// engine.h: This file contains the 'main' function. Program execution begins and ends there.
//

class HUD : public Surface {
public:
	TextBox* debugInfo = new TextBox(0, 0, 100, 100);
	TextBox* inputInfo = new TextBox(250, 0, 100, 100);
	TextBox* drawSurfaceInfo = new TextBox(400, 400, 100, 100);
	TextBox* perfInfo = new TextBox(0, 450, 100, 100);
	void UpdatePerformance();
	HUD();
	void Render(SDL_Renderer* g);
};

#endif