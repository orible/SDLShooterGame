#ifndef CAMERA_H
#define CAMERA_H

#include <SDL.h>
#include "engine.h";

class Camera: public Renderable
{
	float scale = 0;
	int springWindowEnvelope = .8;
	enum Mode {
		FOLLOW_TARGET_SPRING = 1 << 0,
		NORMAL = 1 << 1
	} mode;
	float lerpSpeed = 1;
	Vec2D target;
public:
	void MoveCamera(const Vec2D& v);
	void MoveCameraTowardsSpringy(Vec2D target);
	glm::mat4 GetViewMatrix();
	void Step(double dt, Node* parent);
	void ZoomCamera(float scale);
	void Render(SDL_Renderer* g);
	Camera();
};

#endif // !CAMERA_H