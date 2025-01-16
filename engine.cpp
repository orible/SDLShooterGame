// Game1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include "engine.h"
#include "hud.h"

int Node::GetUID() {
	return uuid++;
}

std::vector<Node*> gctable;
void Node::SetId(std::string Id) {
	this->Id = Id;
}

/// <summary>
	/// Search for Node in graph if it starts with "/" then search from the Root else search from this nodes children
	/// Eg: /PlayHud/Overlay == root -> get child PlayHud -> get Child Overlay -> return Overlay
	/// </summary>
	/// <param name="query"></param>
	/// <returns></returns>
Node* Node::GetNode(std::string query) {
	std::vector<std::string> tokens;
	std::stringstream ss(query);
	std::string token;
	while (std::getline(ss, token, '/')) {
		tokens.push_back(token);
	}
	Node* node = NULL;
	// starts with "/"
	if (tokens[0] == "") {
		node = GetRoot();
	}
	else {
		node = this;
	}
	if (node == NULL) return NULL;
	for (int i = 1; i < tokens.size(); i++) {
		node = node->GetChild(tokens[i]);
		if (node == NULL)
			return NULL;
	}
	return node;
}
Node* Node::GetRoot() {
	Node* node = this;
	while (node->parent != NULL) {
		node = node->parent;
	}
	return node;
}
Node* Node::GetChild(std::string Id) {
	if (this->children.empty()) return NULL;
	for (int i = 0; i < this->children.size(); i++) {
		if (this->children[i]->Id == Id) {
			return this->children[i];
		}
	}
	return NULL;
}
Node* Node::GetParent(std::string Id) {
	if (parent == NULL) {
		return NULL;
	}
	if (parent->Id == Id) {
		return parent;
	}
	parent->GetParent(Id);
}
void Node::Dispose() {
	if (this->isDead) return;
	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->Dispose();
	}
	this->isDead = true;
}
void Node::DoEvent(input_event_args* args) {
	if (this->isDead) return;
	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->DoEvent(args);
	}
}
void Node::SetParent(Node* parent) {
	if (this->isDead) return;
	this->parent = parent;
}
void Node::Step(double dt, Node* parent) {
	if (this->isDead) return;
	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->Step(dt, this);
	}
}
void Node::Render(SDL_Renderer* g) {
	if (this->isDead) return;
	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->Render(g);
	}
}
void Node::AddChild(Node* node) {
	if (this->isDead) return;
	node->parent = this;
	this->children.push_back(node);
}
Node::Node(Node* parent) {
	this->uuid = GetUID();
	this->parent = parent;
	gctable.push_back(this);
}
Node::Node() {
	this->uuid = GetUID();
	this->parent = NULL;
	gctable.push_back(this);
}

void dumpError() {
	printf("SDL_GetError: %s\n", SDL_GetError());
	//SDL_DestroyWindow(window);
	SDL_Quit();
}

long int Node2D::CurTime() {
	return SDL_GetTicks();
}


int Node2D::CurTime_Seconds() {
	return SDL_GetTicks() / 1000.0f;
}


Vec2D Node2D::RotatePoint(Vec2D p, float a) {
	float cosa = cos(a);
	float sina = sin(a);
	return {
		p.x * cosa - p.y * sina,
		p.x * sina + p.y * cosa,
	};
}
	
OOBox Node2D::GetOOBounds() {
	return OOBox{ GetAABounds(), this->rads };
}

Box Node2D::GetAABounds() {
	return { localPos.x, localPos.y, 0, 0 };
}

// get parent transform matrix graph
Transform Node2D::GetGlobalPositionTransform()
{
	Transform transform;
	transform = this->localPos;
	transform.scale = 1;
	
	Node* node = this->parent;
	//if (node == NULL) {
	//	this->globalPos = this->localPos;
	//	transform.x = { this->globalPos, this->rads };
	//	return transform;
	//}

	// traverse node graph backwards and build a transform list
	while (node != NULL) {
		if (node->Type == "NODE2D") {
			Vec2D rotatedTransform = RotatePoint(((Node2D*)node)->localPos, ((Node2D*)node)->rads);
			transform.x += rotatedTransform.x;
			transform.y += rotatedTransform.y;
			//transform = transform + ((Node2D*)node)->GetGlobalPositionTransform();
			//Vec2D transform = this->localPos + ((Node2D*)node)->GetGlobalPositionTransform();
			//this->globalPos = transform;
			//return transform;
		}
		node = node->parent;
	}

	this->globalPos.x = transform.x;
	this->globalPos.y = transform.y;

	//this->globalPos = this->localPos;
	return transform; //this->globalPos;
}

float Node2D::ToScreen(
	double sim_x, double sim_y,
	double* screen_x, double* screen_y,
	Camera camera, int screen_width, int screen_height) {
	*screen_x = (sim_x - camera.x) / camera.scale + screen_width / 2;
	*screen_y = (sim_y - camera.y) / camera.scale + screen_height / 2;
	return 0;
}

void Node2D::RaycastSearch(std::string filter, Vec2D origin, Vec2D dir) {
	//void Box1 = GetAABounds();
	//(Node2D*)nodeRoot;
}

Vec2D Node2D::GetLocalPos() {
	//ToScreen(pos.x, pos.y, &point.x, &point.y, camera, screen.width, screen.height);
	return this->localPos;
}

Node2D::Node2D(): Node()
{
	this->Type = "NODE2D";
}

void Surface::Step(double dt, Node* parent) {
	Node2D::Step(dt, this);
}
void Surface::Render(SDL_Renderer* g) {
	//SDL_Rect rect;
	//SDL_RenderDrawRect(g, &rect);
	Node2D::Render(g);
}

void Sprite::constructTexture(SDL_Renderer* g) {
	if (surf != NULL && texture == NULL) {
		texture = SDL_CreateTextureFromSurface(g, this->surf);
		if (texture == NULL) {
			dumpError();
			return;
		}
		// query deets
		SDL_QueryTexture(texture, NULL, NULL, &width, &height);

		// dump surface out of memory
		SDL_FreeSurface(surf);
	}
}
void Sprite::Step(double dt, Node* parent) {
	this->rads = cos(CurTime() / 1000.f);
	//printf("%d\n", this->rads);
	Node2D::Step(dt, parent);
}
void Sprite::Render(SDL_Renderer* g) {
	// assert texture is constructed all lazy like probably boils to EQ idk, TODO: inspect the assemblr
	constructTexture(g);
	Vec2D transform = GetGlobalPositionTransform();

	//double x = this->globalPos.x;
	//double y = this->globalPos.y;

	//double x = ((Node2D*)this->parent)->pos.x + this->pos.x;
	//double y = ((Node2D*)this->parent)->pos.y + this->pos.y;

	// compute transient fields
	SDL_Rect dstrect = { transform.x, transform.y, width, height };
	SDL_RenderCopyEx(g, this->texture, NULL, &dstrect, this->rads * (100.0f / M_PI), NULL, SDL_FLIP_NONE);
	Node2D::Render(g);
}

Sprite* Sprite::FromDisk(std::string filename) {
	Sprite* sp = new Sprite();
	sp->filename = filename;
	sp->surf = SDL_LoadBMP(("./assets/" + filename).c_str());
	return sp;
}

void Phys2D::do_gravity(double dt) {
	Vec2D v1 = { 0, -100 };
	Vec2D v2 = GetGlobalPositionTransform(); //this->localPos;
	Vec2D d = { v2.x - v1.x, v2.y - v1.y };

	float m1 = 10;
	float m2 = 10;

	float dist = sqrt(d.x * d.x + d.y * d.y);

	float f = G * m1 * m2 / (dist * dist);

	Vec2D dv = { 0, 0 };
	if (dist > 0.0f) {
		dv = { d.x / dist, d.y / dist };
	}

	Vec2D Fv = { f * dv.x, f * dv.y };

	// accelleration
	Vec2D accel1 = { Fv.x / m1, Fv.y / m1 };

	// add to velocity  vector
	velocity.x += accel1.x * dt;
	velocity.y += accel1.y * dt;
}

// compute friction
void Phys2D::do_friction(double dt) {
	// SQRT(v.x^2 + v.y^2)
	double speed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
	if (speed > 0) {
		double decel = friction * 9.8 * dt;
		double new_speed = fmax(0, speed - decel);
		velocity.x *= new_speed / speed;
		velocity.y *= new_speed / speed;
	}
}

void Phys2D::Step(double dt, Node* parent) {
	GetGlobalPositionTransform();
	double mag = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
	if (mag > 0) {
		// break
		int a = 0;
	}

	do_gravity(dt);

	do_friction(dt);

	localPos.x += velocity.x * dt;
	localPos.y += velocity.y * dt;

	((HUD*)this->GetNode("/PlayHUD"))->debugInfo->SetTextF(
		"Pos: %.1f x %.1f\nVel: %.1f x %.1f\nMag: %f",
		localPos.x, localPos.y,
		velocity.x, velocity.y,
		mag);
	
	//velocity.x += acceleration.x * dt;
	//velocity.y += acceleration.y * dt;
	Node2D::Step(dt, this);
}

void Node2D_Test::Step(double dt, Node* parent)
{
	//this->localPos.x += cos(CurTime() / 1000.f) * 100.0f * dt;
	//this->localPos.y += cos(CurTime() / 1000.f) * 100.0f * dt;
	//this->rads = cos(CurTime() / 1000.f);
	Node2D::Step(dt, parent);
}
