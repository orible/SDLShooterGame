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
} CameraData;

typedef struct {
	int width, height;
} Screen;

typedef struct {
	double x, y;
} Point;

typedef struct Vec2D {
	double x, y;
	Vec2D operator -(Vec2D a) {
		return { this->x - a.x, this->y- a.y };
	}
	Vec2D operator +(Vec2D a) {
		return { this->x + a.x, this->y + a.y };
	}
	Vec2D operator *(Vec2D a) {
		return { this->x * a.x, this->y * a.y };
	}
	float length() const {
		return sqrt(x * x + y * y);
	}
} Vec2D;

typedef struct Transform: Vec2D {
	float rads;
	float scale;
	void operator =(Vec2D a) {
		this->x = a.x;
		this->y = a.y;
	};
} Transform;

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
	std::string Type;
	bool isDead = false;
	NodeType type = NodeType::NODE;
	Node* parent;
	std::vector<Node*> children;
	int GetUID();
	void SetId(std::string Id);
	void AddChild(Node* node);
	void SetParent(Node* parent);

	// called when added as child to another node
	virtual void OnAddedToTree(Node* caller);

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
	virtual void RenderGraph(SDL_Renderer* g);
	Node(Node* parent);
	Node();
};

void dumpError();

class Node2D : public Node {
public:
	float rads;
	Vec2D localPos;
	Vec2D globalPos;

	long int CurTime();
	int CurTime_Seconds();
	Vec2D RotatePoint(Vec2D p, float a);
	virtual OOBox GetOOBounds();
	virtual Box GetAABounds();
	Transform GetGlobalPositionTransform();
	void SetLocalPos(Vec2D p);
	float ToScreen(
		double sim_x, double sim_y,
		double* screen_x, double* screen_y,
		CameraData camera, int screen_width, int screen_height);
	void RaycastSearch(std::string filter, Vec2D origin, Vec2D dir);
	Vec2D GetLocalPos();
	Node2D();
};

class Node2D_Test : public Node2D {
public:
	void Step(double dt, Node* parent);
};

class Surface : public Node2D {
protected:
	int texwidth;
	int texheight;
	int width; 
	int height;
public:
	void Step(double dt, Node* parent);
	void Render(SDL_Renderer* g);
};

class Sprite : public Surface {
	//int width;
	//int height;
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