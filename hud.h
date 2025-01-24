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
	TextBox* perfInfo = new TextBox(0, 430, 100, 100);
	TextBox* logo = new TextBox(0, 0, 0, 0);
	TextBox* trackInfo = new TextBox(0, 0, 0, 0);

	void UpdatePerformance();
	void OnAddedToTree(Node* caller);
	HUD();
	void Render(SDL_Renderer* g);
};

#endif