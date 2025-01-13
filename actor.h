#ifndef H_ACTOR
#define H_ACTOR
#include "engine.h";
// engine.h: This file contains the 'main' function. Program execution begins and ends there.
//
class Actor : public Phys2D {
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
public:
	/// <summary>
	/// read keys in event buffer
	/// </summary>
	/// <param name="event"></param>
	/// <returns></returns>
	bool computeInputVectors(SDL_Event event);

	void DoEvent(input_event_args* args);
	void Step(double dt, Node* parent);

	Actor();

	//Sprite * sprite = new Sprite();
};

#endif