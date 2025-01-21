#ifndef CAMERA_H
#define CAMERA_H

#include <SDL.h>
#include "engine.h";

class Camera: public Node2D
{
	float scale = 0;
public:
	void MoveCamera(Vec2D v);
	void ZoomCamera(float scale);
	void Render(SDL_Renderer* g);
	Camera();
};

#endif // !CAMERA_H