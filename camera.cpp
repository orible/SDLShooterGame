#include "engine.h"
#include "camera.h"
#include "hud.h"
#include "system.h"

void Camera::MoveCamera(const Vec2D& v)
{
	this->localPos = this->localPos - v;
}

void Camera::MoveCameraTowardsSpringyWorld(Vec2D target)
{
	// set camera movement target!
	this->target = target;
}

glm::mat4 Camera::GetViewMatrix() {
	return glm::inverse(this->GetWorldTransform());
}

//return glm::translate(glm::mat4(1.0f), glm::vec3(-localPos, 0.0f));

bool Camera::GetInFustrum(Renderable* r)
{
	
	return false;
}

void Camera::_OnStep(double dt, Node* parent) {
	// lookup screen width from root node
	System * sys = (System*)this->GetNode("/Root");
	
	//const Vec2D offset = Vec2D{ sys->ScreenWidth / 2.0f, sys->ScreenHeight / 2.0f };

	if (mode == Mode::FOLLOW_TARGET_SPRING) {
		float t = lerpSpeed * dt;
		if (t > 1.0f) {
			t = 1.0f;
		}
		glm::vec2 camWorld = GetWorldPos();

		Vec2D cam = GetWorldPos();
		glm::vec2 desiredWorld = lerp(camWorld, target, t);
		if (this->parent)
		{
			// invert parent World-to-Local
			glm::mat4 invParent = glm::inverse(((Node2D*)this->parent)->GetWorldTransform());
			glm::vec4 local = invParent * glm::vec4(desiredWorld, 0.0f, 1.0f);
			this->localPos = glm::vec2(local);
		}
		else
		{
			this->localPos = desiredWorld;
		}
		//this->localPos = lerp(this->localPos, target, t);
	}

	((HUD*)this->GetNode("*/DebugHUD"))
		->camInfo
		->SetTextF("Cam: (x: %f) (y: %f), tgt: (%f, %f)", 
			this->localPos.x, this->localPos.y, 
			this->target.x, this->target.y);
}

void Camera::ZoomCamera(float scale)
{
	this->scale = scale;
}

void Camera::OnCreated()
{
	this->SetId("Camera");
	this->mode = Mode::FOLLOW_TARGET_SPRING;
	this->flags = Node2D::NodeFlags::NONE; //Node2D::NodeFlags::SKIP_TRANSFORM;
}
