#ifndef CAMERA_H
#define CAMERA_H

#include <SDL.h>
#include "engine.h";

class Camera: Node2D
{
	void MoveCamera(Vec2D v);
	void ZoomCamera(float scale);
};

#endif // !CAMERA_H