#ifndef H_ACTOR
#define H_ACTOR
#include "engine.h"
#include "controls.h"
#include "sprite.h"

// engine.h: This file contains the 'main' function. Program execution begins and ends there.
//
DECLARE_NODE(Actor, Phys2D)
	Sprite* sprite;
	Vec2D dir = { 0, 0 };
	Vec2D dirn = { 0, 0 };
	enum Actions {
		UP = 0,
		DOWN = 1,
		LEFT = 2,
		RIGHT = 3,
		SHOOT = 4,
		JUMP = 5,
	};
	enum Mode {
		INACTIVE = 0x0,
		PRESSED = 0x1,
	};
	int actions[5] = {
		INACTIVE, INACTIVE, INACTIVE ,INACTIVE, INACTIVE
	};
	TextBox* info = new TextBox(1, 1, 100, 100);
public:
	/// read keys in event buffer
	bool computeInputVectors(SDL_Event event);
	void DoEvent(input_event_args* args);
	HOOK(void, OnStep, (double dt, Node* parent), (dt, parent));
	void OnCreated();
END_DECLARE_NODE();

#endif