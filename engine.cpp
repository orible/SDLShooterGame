// Game1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "engine.h"
#include "hud.h"
#include <assert.h>
#include "system.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


int InitLibs() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return -1;
	}

	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		return -1;
	}

	TTF_Init();

	return 1;
}

// Normalize between -π and π
float NormalizeRotation(float rads) {
	const float PI = M_PI;
	while (rads >= PI) rads -= 2 * PI;
	while (rads < -PI) rads += 2 * PI;
	return rads;
}

// Ensure the angle is between 0 and 2*π (0 to 360 degrees)
float ClampRotation(float rads) {
	const float TWO_PI = 2 * M_PI;
	while (rads >= TWO_PI) rads -= TWO_PI;   // Wrap around for angles greater than 2π
	while (rads < 0) rads += TWO_PI;          // Wrap around for negative angles
	return rads;
}

int Node::GetUID() {
	return uuid++;
}

std::vector<Node*> gctable;
void Node::SetId(std::string Id) {
	// illegal to contain '/' characters so the user (me) doesn't confuse themselves
	assert(Id.find('/') == std::string::npos);
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
		if (tokens.size() == 2 && node->Id == tokens[1]) {
			return node;
		}
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
/// <summary>
/// Gets the highest ancestor in the node tree
/// If this has no parent it returns NULL!
/// </summary>
/// <returns></returns>
Node* Node::GetRoot() {
	if (root_Cached != NULL) {
		return this->root_Cached;
	}

	// if no parent then just return null!
	if (this->parent == NULL) {
		return NULL;
	}

	Node* node = this;
	while (node->parent != NULL) {
		node = node->parent;
	}

	this->root_Cached = node;
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
bool Node::InheritsFrom(const std::string& str) 
{
	return std::find(this->nodetable.begin(), this->nodetable.end(), str) != this->nodetable.end();
}
void Node::OnAddedToTree(Node* parent)
{
	// do nothing
}
void Node::Step(double dt, Node* parent) {
	if (this->isDead) return;
	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->Step(dt, this);
	}
}
/*
void Node::Render(RenderParams* p) {
	// do nothing is a stubby
}
void Node::RenderGraph(RenderParams* p) {
	if (this->isDead) return;
	this->Render(p);
	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->RenderGraph(p);
	}
}*/

Node::~Node()
{
	this->Dispose();
}

void Node::AddChild(Node* node) {
	if (this->isDead) return;
	node->parent = this;
	this->children.push_back(node);
	node->OnAddedToTree(this);
}
void Node::RemoveChild(NodeRef ref)
{
	this->children.erase(std::remove(this->children.begin(), this->children.end(), ref), this->children.end());
}
void Node::RemoveSelf()
{
	this->parent->RemoveChild(this);
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

glm::vec2 Node2D::GetPositionFromMatrix(const glm::mat4& matrix)
{
	return glm::vec2(matrix[3].x, matrix[3].y);
}

Vec2D Node2D::RotatePoint(Vec2D p, float a) {
	float cosa = cos(a);
	float sina = sin(a);
	return {
		p.x * cosa - p.y * sina,
		p.x * sina + p.y * cosa,
	};
}

void Node2D::GetLocalTransform()
{
	localTransform = glm::translate(glm::mat4(1.0f), glm::vec3(localPos, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), rads, glm::vec3(0, 0, 1)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f));
}
	
OBB Solid2D::GetOOBounds() {
	Vec2D pos = this->GetLocalPos();
	Vec2D c{ pos.x + (this->bounds.width / 2), pos.y + (this->bounds.height / 2) };
	Vec2D v{ (this->bounds.width / 2), (this->bounds.height / 2) };
	return OBB{ c, v, this->rads };
}

Box Solid2D::GetAABounds() {
	return { }; //localPos.x, localPos.y, 0, 0 };
}

bool Solid2D::ShouldRender()
{
	return true;
}

glm::mat4 Renderable::GetGlobalPositionTransformWithCamera() {
	Node* node = this->parent;

	glm::mat4 mat = localTransform;

	std::vector<glm::mat4> transforms;
	// traverse node graph backwards and build a transform list
	while (node != NULL) {
		if (node->InheritsFrom(Node2D::GetTypeName())) {
			Node2D * nd = (Node2D*)node;
			transforms.push_back(nd->localTransform);
		}
		node = node->parent;
	}

	for (int i = transforms.size() - 1; i >= 0; i--) {
		mat = transforms[i] * mat;
	}

	return mat;
}

glm::mat4 Node2D::GetGlobalPositionTransform() {
	Node* node = this->parent;

	while (node != NULL) {
		if (node->InheritsFrom(Node2D::GetTypeName())) {
			Node2D* nd = (Node2D*)node;
			nd->InheritsFrom(Node::ClassName());
			//return parent->GetGlobalPositionTransform() * this->GetLocalTransform();
		}
		node = node->parent;
	}

	// return GetLocalTransform();


	glm::mat4 mat = localTransform;
	std::vector<glm::mat4> transforms;
	// traverse node graph backwards and build a transform list
	while (node != NULL) {
		if (node->InheritsFrom(Node2D::GetTypeName())) {
			Node2D* nd = (Node2D*)node;
			transforms.push_back(nd->localTransform);
		}
		node = node->parent;
	}

	for (int i = transforms.size() - 1; i >= 0; i--) {
		mat = transforms[i] * mat;
	}

	return mat;
}

/*
// get parent transform matrix graph
// DOES NOT include the camera object if one is found!
glm::mat4 Node2D::GetGlobalPositionTransform()
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
			if (((Node2D*)node)->flags & NodeFlags::SKIP_TRANSFORM) {
				// skip flag set so skip it
				node = node->parent;
				continue;
			}
			if (((Node2D*)node)->Id == "Camera") {
				// do not factor the camera into the transform
				node = node->parent;
				continue;
			}
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
	return glm::mat4(1.0f); //this->globalPos;
}*/

float Node2D::GetLocalRotation() {
	return rads;
}

float Node2D::GetGlobalRotation()
{
	Node* node = this->parent;
	float rads = this->rads;
	while (node != NULL) {
		if (node->InheritsFrom(Node2D::GetTypeName())) {
			rads += ((Node2D*)node)->rads;
		}
		node = node->parent;
	}

	return rads;
}

void Node2D::SetLocalPos(Vec2D p)
{
	this->localPos = p;
}

float Node2D::ToScreen(
	double sim_x, double sim_y,
	double* screen_x, double* screen_y,
	CameraData camera, int screen_width, int screen_height) {
	*screen_x = (sim_x - camera.x) / camera.scale + screen_width / 2;
	*screen_y = (sim_y - camera.y) / camera.scale + screen_height / 2;
	return 0;
}

bool Solid2D::ShouldCollide(Solid2D* caller)
{
	// called by another ray intersection test - override if true
	return true;
}

bool Solid2D::RayIntersectsOBB(const Ray& ray, const OBB& obb, float& tEntry, float& tExit) {
	// Step 1: Compute the local-space ray
	/*Vec2D rayOriginLocal = (ray.origin - obb.center).Rotate(-obb.rotation);
	Vec2D rayDirectionLocal = ray.direction.Rotate(-obb.rotation);

	// Step 2: Perform AABB-Ray intersection in local space
	Vec2D min = { -obb.halfExtents.x, -obb.halfExtents.y };
	Vec2D max = { obb.halfExtents.x, obb.halfExtents.y };

	float invDirX = 1.0f / rayDirectionLocal.x;
	float invDirY = 1.0f / rayDirectionLocal.y;

	float tMinX = (min.x - rayOriginLocal.x) * invDirX;
	float tMaxX = (max.x - rayOriginLocal.x) * invDirX;
	if (invDirX < 0.0f) std::swap(tMinX, tMaxX);

	float tMinY = (min.y - rayOriginLocal.y) * invDirY;
	float tMaxY = (max.y - rayOriginLocal.y) * invDirY;
	if (invDirY < 0.0f) std::swap(tMinY, tMaxY);

	tEntry = std::max(tMinX, tMinY);
	tExit = std::min(tMaxX, tMaxY);

	// Step 3: Check if intersection exists
	return tEntry <= tExit && tExit > 0.0f;*/
	return false;
}

void Solid2D::RaycastSearch(std::string filter, Vec2D origin, Vec2D dir) {
	//void Box1 = GetAABounds();
	//(Node2D*)nodeRoot;
}

bool Node2D::ShouldRender()
{
	return true;
}

/*
void Node2D::Render(RenderParams* p)
{
	Node::Render(p);
}*/

Vec2D Node2D::GetLocalPos() {
	//ToScreen(pos.x, pos.y, &point.x, &point.y, camera, screen.width, screen.height);
	return this->localPos;
}

void Renderable::Step(double dt, Node* parent) {
	Node2D::Step(dt, this);
}

void Renderable::OnRender(RenderParams* p)
{
	//throw "stubby";
}

void Renderable::Render(RenderParams* p)
{
	GetGlobalPositionTransformWithCamera();
	this->OnRender(p);
}

void Sprite::constructTexture(SDL_Renderer* g) {
	if (surf != NULL && texture == NULL) {
		texture = SDL_CreateTextureFromSurface(g, this->surf);
		if (texture == NULL) {
			dumpError();
			return;
		}
		// query deets
		UpdateTextureInfo();

		// dump surface out of memory
		SDL_FreeSurface(surf);
	}
}

Box Sprite::GetSpriteSize()
{
	constructTexture(((System*)this->GetRoot())->GetRenderParams().g);

	// TODO: fix this by using box not vec2D as it's narrowing an int to double!!
	return Box{ Vec2D{0, 0}, this->texwidth, this->texheight };
}

Vec2D Node2D::lerp(const Vec2D& a, const Vec2D& b, float t)
{
	return a + (b - a) * t;
}

/// <summary>
///	Read sprite memory and copy part of or all of it's texture
/// </summary>
/// <param name="sprite"></param>
/// <returns></returns>
Sprite* Sprite::CopyRegion(Box p)
{
	SDL_Renderer *g = ((System*)this->GetRoot())->GetRenderParams().g;
	SDL_Texture *tex = SDL_CreateTexture(g, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, p.width, p.height);

	SDL_SetRenderTarget(g, tex);
	SDL_SetRenderDrawColor(g, 100, 100, 100, 0);
	SDL_RenderClear(g);

	SDL_Rect r{ p.pos.x, p.pos.y, p.width, p.height };
	SDL_RenderCopy(g, this->texture, &r, NULL);
	
	SDL_SetRenderTarget(g, NULL);

	Sprite *sp = new Sprite();
	sp->texture = tex;
	sp->UpdateTextureInfo();

	return sp;
}

void Sprite::SamplePoints(std::vector<Vec2D> *p)
{
	SDL_SetTextureBlendMode(this->texture, SDL_BLENDMODE_BLEND);
	
	int width, height;
	SDL_QueryTexture(this->texture, NULL, NULL, &width, &height);

	int pitch;
	void* pixels;
	SDL_LockTexture(this->texture, NULL, &pixels, &pitch);

	Uint32* upixels = (Uint32*)pixels;

	//for (int i = 0; i < width * height; i++) {}

	SDL_UnlockTexture(this->texture);
}
void Sprite::Step(double dt, Node* parent) {
	//this->rads = cos(CurTime() / 1000.f);
	//printf("%d\n", this->rads);
	Node2D::Step(dt, parent);
}
void Sprite::OnRender(RenderParams *p) {
	// assert texture is constructed all lazy like probably boils to EQ idk, TODO: inspect the assemblr
	constructTexture(p->g);
	glm::mat4 transform = GetGlobalPositionTransformWithCamera();

	//double x = this->globalPos.x;
	//double y = this->globalPos.y;

	//double x = ((Node2D*)this->parent)->pos.x + this->pos.x;
	//double y = ((Node2D*)this->parent)->pos.y + this->pos.y;

	glm::vec3 p1 = glm::vec3(transform[3]);

	// compute transient fields
	SDL_Rect dstrect = { p1.x, p1.y, this->texwidth, this->texheight };
	SDL_RenderCopyEx(p->g, this->texture, NULL, &dstrect, this->rads * (100.0f / M_PI), NULL, SDL_FLIP_NONE);
}

void Sprite::UpdateTextureInfo()
{
	SDL_QueryTexture(texture, NULL, NULL, &texwidth, &texheight);
}

void Sprite::Dispose()
{
	if (this->texture != NULL) {
		SDL_DestroyTexture(this->texture);
	}
}

Sprite* Sprite::FromDisk(std::string filename) {
	Sprite* sp = new Sprite();
	sp->filename = filename;
	sp->surf = IMG_Load(("./assets/" + filename).c_str());
	//sp->surf = SDL_LoadBMP(("./assets/" + filename).c_str());
	return sp;
}

void Phys2D::do_gravity(double dt) {
	//Vec2D v1 = { 0, -100 };
	Vec2D v1 = { 0, -100 };
	//Vec2D v2 = GetGlobalPositionTransform(); //this->localPos;

	glm::mat4 mat= GetGlobalPositionTransform();

	glm::vec3 v2(mat[3]);


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

	((HUD*)this->GetNode("/DebugHUD"))->debugInfo->SetTextF(
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

SDL_Texture* RenderParams::Layer(Box* bhint)
{
	return nullptr;
}
