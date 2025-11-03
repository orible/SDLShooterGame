#ifndef CAMERA_H
#define CAMERA_H

#include <SDL.h>
#include "engine.h";

DECLARE_NODE(Camera, Renderable)
	float scale = 0;
	int springWindowEnvelope = .8;
	enum Mode {
		FOLLOW_TARGET_SPRING = 1 << 0,
		NORMAL = 1 << 1
	} mode;
	float lerpSpeed = 1;
	Vec2D target = Vec2D{ 0, 0 };
public:
	void MoveCamera(const Vec2D& v);
	void MoveCameraTowardsSpringyWorld(Vec2D target);
	glm::mat4 GetViewMatrix();

	// TODO: THIS FEELS CIRCULAR LOL
	bool GetInFustrum(Renderable* r);

	HOOK(void, OnStep, (double dt, Node* parent), (dt, parent));

	//void Step(double dt, Node* parent);
	void ZoomCamera(float scale);
	//void Render(SDL_Renderer* g);
	//HOOK(void, Render, (SDL_Renderer* g), (g));
	void OnCreated();
	//Camera();
};

#endif // !CAMERA_H