#ifndef ENGINE_H
#define ENGINE_H
// engine.h: This file contains the 'main' function. Program execution begins and ends there.
//

#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

typedef struct {
	double x, y;        // Center of the viewport in simulation space
	double scale;       // Scale factor: simulation units per screen pixel
} Camera;

typedef struct {
	int width, height;
} Screen;

typedef struct {
	double x, y;
} Point;

typedef struct Vec2D {
	double x, y;
	float length() const {
		return sqrt(x * x + y * y);
	}
} Vec2D;

typedef struct VecInt2D {
	int x, y;
	float length() const {
		return sqrt(x * x + y * y);
	}
} VecInt2D;

typedef struct {
	Vec2D pos;
	int width;
	int height;
} Box;

typedef struct {
	Box box;
	float rot;
} OOBox;

enum NodeType {
	NODE,
	NODE2D,
	SURFACE,
	SPRITE,
	PHYS,
};

typedef struct {
	SDL_Event event;
	long int timestamp;
} input_event;
typedef std::vector<SDL_Event> eventbuf;

typedef struct {
	eventbuf* buf;
	SDL_Event ev;
	Vec2D dir;
} input_event_args;

class Node {
public:
	int uuid;
	std::string Id;
	bool isDead = false;
	NodeType type = NodeType::NODE;
	Node* parent;
	std::vector<Node*> children;
	int GetUID();
	void SetId(std::string Id);
	void AddChild(Node* node);
	void SetParent(Node* parent);
	/// <summary>
	/// Search for Node in graph if it starts with "/" then search from the Root else search from this nodes children
	/// Eg: /PlayHud/Overlay == root -> get child PlayHud -> get Child Overlay -> return Overlay
	/// </summary>
	/// <param name="query"></param>
	/// <returns></returns>
	Node* GetNode(std::string query);
	Node* GetRoot();
	Node* GetChild(std::string Id);
	Node* GetParent(std::string Id);
	virtual void Dispose();
	virtual void DoEvent(input_event_args* args);
	virtual void Step(double dt, Node* parent);
	virtual void Render(SDL_Renderer* g);

	Node(Node* parent);
	Node();
};

void dumpError();

class Node2D : public Node {
public:
	float rads;
	Vec2D pos;
	long int CurTime();
	Vec2D RotatePoint(Vec2D p, float a);
	virtual OOBox GetOOBounds();
	virtual Box GetAABounds();

	float ToScreen(
		double sim_x, double sim_y,
		double* screen_x, double* screen_y,
		Camera camera, int screen_width, int screen_height);
	void RaycastSearch(std::string filter, Vec2D origin, Vec2D dir);
	Point GetTransform();
};

class Surface : public Node2D {
public:
	void Step(double dt, Node* parent);
	void Render(SDL_Renderer* g);
};

class Sprite : public Node2D {
	int width;
	int height;
	SDL_Surface* surf;
	SDL_Texture* texture;
	std::string filename;
public:
	void constructTexture(SDL_Renderer* g);
	void Step(double dt, Node* parent);
	void Render(SDL_Renderer* g);
	static Sprite* FromDisk(std::string filename);
};

class Phys2D : public Node2D {
public:
	double friction = 10;
	Vec2D velocity;
	Vec2D acceleration;
	const float G = 10000; //6.674;
	// compute gravity
	void do_gravity(double dt);
	// compute friction
	void do_friction(double dt);
	void Step(double dt, Node* parent);
};

#endif