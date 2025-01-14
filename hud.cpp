#include "hud.h"
#include <SDL.h>
#include "engine.h"

void HUD::UpdatePerformance() {
	debugInfo->SetText("Hello!");
}

HUD::HUD() : Surface()
{
	this->AddChild(this->debugInfo);
	this->AddChild(this->inputInfo);
	this->AddChild(this->drawSurfaceInfo);
}

void HUD::Render(SDL_Renderer* g) {
	//debugInfo->Render(g);
	//inputInfo->Render(g);
	//drawSurfaceInfo->Render(g);
	Surface::Render(g);
}