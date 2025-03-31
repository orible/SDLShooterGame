#include "engine.h"
#include "camera.h"
#include "hud.h"
#include "system.h"

void Camera::MoveCamera(const Vec2D& v)
{
	this->localPos = this->localPos - v;
}

void Camera::MoveCameraTowardsSpringy(Vec2D target)
{
	this->target = target;
}

glm::mat4 Camera::GetViewMatrix() {
	return glm::translate(glm::mat4(1.0f), glm::vec3(-globalPos, 0.0f));
}

void Camera::Step(double dt, Node* parent) {
	System * sys = (System*)this->GetNode("/Root");
	const Vec2D offset = Vec2D{ sys->ScreenWidth / 2.0f, sys->ScreenHeight / 2.0f };

	sys->ScreenWidth;

	if (mode == Mode::FOLLOW_TARGET_SPRING) {
		float t = lerpSpeed * dt;
		if (t > 1.0f) {
			t = 1.0f;
		}
		this->localPos = lerp(this->localPos, target - offset, t);
	}

	//this->localPos = target; //(target - offset);

	((HUD*)this->GetNode("/DebugHUD"))
		->camInfo
		->SetTextF("Cam: (x: %f) (y: %f)", this->localPos.x, this->localPos.y);

	Renderable::Step(dt, parent);
}
void Camera::ZoomCamera(float scale)
{
	this->scale = scale;
}

Camera::Camera()
{
	this->SetId("Camera");
	this->mode = Mode::FOLLOW_TARGET_SPRING;
	this->flags = Node2D::NodeFlags::NONE; //Node2D::NodeFlags::SKIP_TRANSFORM;
}
