#include "camera.h"

void Camera::MoveCamera(Vec2D v)
{
	this->localPos = this->localPos - v;
}

void Camera::ZoomCamera(float scale)
{
	this->scale = scale;
}

void Camera::Render(SDL_Renderer* g)
{
	Node2D::Render(g);
}

Camera::Camera()
{
	this->SetId("Camera");
}
