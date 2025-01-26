#include "camera.h"

void Camera::MoveCamera(Vec2D v)
{
	this->localPos = this->localPos - v;
}

void Camera::ZoomCamera(float scale)
{
	this->scale = scale;
}

Camera::Camera()
{
	this->SetId("Camera");
}
