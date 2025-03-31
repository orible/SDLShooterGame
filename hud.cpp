#include "hud.h"
#include <SDL.h>
#include "engine.h"
#include "system.h"

void HUD::UpdatePerformance() {
	debugInfo->SetText("Hello!");
}

void HUD::OnAddedToTree(Node * caller) {
	System* sys = (System*)this->GetRoot();
	this->logo->SetText("Rhythm Runners");
	Box sz = sys->GetWindowSize("main");
	this->logo->localPos.x = sz.width - 260;
	this->logo->localPos.y = sz.height - 40;

	Vec2D v{ 0, sys->ScreenHeight - 80 };
	this->inputInfo->SetLocalPos(v);

	this->trackInfo->SetLocalPos(Vec2D{ 
		(double)sys->ScreenWidth - 300, 
		100 
	});
	this->trackInfo->SetFontSize(48);
}

HUD::HUD() : Renderable()
{
	//this->AddChild(this->perfInfo);
	this->AddChild(this->debugInfo);
	this->AddChild(this->inputInfo);
	this->AddChild(this->drawSurfaceInfo);
	this->AddChild(this->trackInfo);
	this->AddChild(this->camInfo);
}

void HUD::OnRender(RenderParams* p)
{
	perfInfo->Render(p);
	logo->Render(p);
	//debugInfo->Render(g);
	//inputInfo->Render(g);
	//drawSurfaceInfo->Render(g);
}