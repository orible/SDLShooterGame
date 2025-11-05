#include "hud.h"
#include <SDL.h>
#include "engine.h"
#include "system.h"

Vec2D FromScreen(Box scr, float x, float y)
{
	return Vec2D{ scr.width * x, scr.height * y };
}

void HUD::UpdatePerformance() {
	debugInfo->SetText("Hello!");
}

void HUD::_OnAddedToTree(Node * caller) {
	System* sys = (System*)this->GetRoot();


	this->logo->SetTextF("DEEPSTATE (%.f)", 0.1f);
	Box sz = sys->GetWindowSize("main");

	this->logo->SetPos(FromScreen(sz, .1, .1));
	this->inputInfo->SetPos(FromScreen(sz, .1, .2));
	this->trackInfo->SetPos(FromScreen(sz, .1, .3));
	this->perfInfo->SetPos(FromScreen(sz, .1, .4));
	this->trackInfo->SetFontSize(48);
	this->AddChild(logo);
	//super::OnAddedToTree(caller);
}

void HUD::_OnCreated()
{
	this->index = 9999;
	this->AddChild(this->perfInfo);
	this->AddChild(this->debugInfo);
	this->AddChild(this->inputInfo);
	this->AddChild(this->drawSurfaceInfo);
	this->AddChild(this->trackInfo);
	this->AddChild(this->camInfo);
}