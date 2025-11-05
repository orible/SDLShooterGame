#include "actor.h"
#include <SDL.h>
#include "engine.h"
#include "hud.h"
#include "surfacedrawable.h"
#include "camera.h"
#include "sprite.h"

bool Actor::computeInputVectors(SDL_Event event) {
	switch (event.type) {
	case SDL_KEYDOWN:
		// called repeatedly for rollover
		//dir = { 0, 0 };
		switch (event.key.keysym.sym) {
		case SDLK_w:
			actions[Actions::UP] = Mode::PRESSED;
			//dir.y -= 1.0f;  // Move up
			break;
		case SDLK_a:
			actions[Actions::LEFT] = Mode::PRESSED;
			//dir.x -= 1.0f;  // Move left
			break;
		case SDLK_s:
			actions[Actions::DOWN] = Mode::PRESSED;
			//dir.y += 1.0f;  // Move down
			break;
		case SDLK_d:
			actions[Actions::RIGHT] = Mode::PRESSED;
			//dir.x += 1.0f;  // Move right
			break;
		}
		break;
	case SDL_KEYUP:
		switch (event.key.keysym.sym) {
		case SDLK_w:
			actions[Actions::UP] = Mode::INACTIVE;
			//dir.y += 1.0f;  // Stop moving up
			break;
		case SDLK_a:
			actions[Actions::LEFT] = Mode::INACTIVE;
			//dir.x += 1.0f;  // Stop moving left
			break;
		case SDLK_s:
			actions[Actions::DOWN] = Mode::INACTIVE;
			//dir.y -= 1.0f;  // Stop moving down
			break;
		case SDLK_d:
			actions[Actions::RIGHT] = Mode::INACTIVE;
			//dir.x -= 1.0f;  // Stop moving right
			break;
		}
		break;/**/
	default:
		return false;
	}

	/*Vec2D dirn = {dir.x, dir.y};

	// Normalize the movement vector (to prevent faster diagonal movement)
	float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
	if (length > 0.0f) {
		dirn.x = dir.x / length;
		dirn.y = dir.y /length;
	}

	printf("%f\n", length);
	return length > 0.0f;*/
	return 0;
}

void Actor::DoEvent(input_event_args* args) {
	computeInputVectors(args->ev);
	Phys2D::DoEvent(args);
}

void Actor::_OnStep(double dt, Node* parent) {
	dir.x = (actions[Actions::RIGHT] ? 1.0f : 0.0f) - (actions[Actions::LEFT] ? 1.0f : 0.0f); // Right - Left
	dir.y = (actions[Actions::DOWN] ? 1.0f : 0.0f) - (actions[Actions::UP] ? 1.0f : 0.0f); // Down - Up

	this->info->SetTextF("X/Y: %f, %f", this->localPos.x, this->localPos.y);
	float mag = sqrtf(dir.x * dir.x + dir.y * dir.y);
	HUD*hud = ((HUD*)this->GetNode("/*/DebugHUD"));
	hud->inputInfo->SetTextF("W[%c] S[%c] A[%c] D[%c] SPACE[?] %f\nAxis: (%f,%f)",
		this->dir.y < 0 ? 'X' : '_',
		this->dir.y > 0 ? 'X' : '_',
		this->dir.x < 0 ? 'X' : '_',
		this->dir.x > 0 ? 'X' : '_', mag, dir.x, dir.y);

	const float impulse = 10.0f;

	float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
	if (length > 0.0000001f) {
		dir.x /= length;
		dir.y /= length;

		this->velocity.x += dir.x * impulse;
		this->velocity.y += dir.y * impulse;
	}

	Camera * cam = ((Camera*)this->GetNode("/*/Camera"));
	cam->MoveCameraTowardsSpringyWorld(GetWorldPos());
}

void Actor::OnCreated() {
	this->sprite = Sprite::FromDisk("./player.bmp");
	// this will set the sprite parent and hook it as a child that is auto executed on step, render etc
	this->AddChild(sprite);
	this->AddChild(info);
}