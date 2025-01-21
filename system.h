#pragma once

#include "engine.h"

class System: public Node2D
{
public:
	int ScreenHeight;
	int ScreenWidth;
	int ResizeWindow(int x, int y);
};

