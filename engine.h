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

struct RenderParams {
	SDL_Renderer* g;
};

// Normalize between -π and π
float NormalizeRotation(float rads);

// Ensure the angle is between 0 and 2*π (0 to 360 degrees)
float ClampRotation(float rads);

typedef struct Vec2D {
	double x, y;
	Vec2D operator -(Vec2D a) {
		return { this->x - a.x, this->y- a.y };
	}
	Vec2D operator +(Vec2D a) {
		return { this->x + a.x, this->y + a.y };
	}
	Vec2D operator *(float scalar) const {
		return { x * scalar, y * scalar };
	}
	// Vector subtraction
	Vec2D operator-(const Vec2D& other) const {
		return { x - other.x, y - other.y };
	}
	Vec2D Hadamard(Vec2D a) {
		return { this->x * a.x, this->y * a.y };
	}
	float Cross(Vec2D a) {
		return this->x * a.y - this->y * a.x;
	}
	float length() const {
		return sqrt(x * x + y * y);
	}

	/// <summary>
	/// the measurement of how perpendicular a vector is to another one, if length of a or b vectors is 0 then the result is zero!
	/// if the vector is the same or the same but mirrored the result is zero!
	/// (1, 1) x (-1, -1) == 0	 
	/// dot = 0: Vectors are perpendicular.
	/// dot > 0: Vectors are pointing in the same general direction.
	/// dot < 0 : Vectors are pointing in opposite directions.
	/// </summary>
	/// <param name="other"></param>
	/// <returns></returns>
	float Dot(const Vec2D& other) const {
		return x * other.x + y * other.y;
	}

	// Rotate vector by an angle (radians)
	Vec2D Rotate(float angle) const {
		float cosTheta = std::cos(angle);
		float sinTheta = std::sin(angle);
		return {
			x * cosTheta - y * sinTheta,
			x * sinTheta + y * cosTheta
		};
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

struct Box {
	Vec2D pos;
	int width;
	int height;
	void Zero() {
		this->pos.x = 0;
		this->pos.y = 0;
		this->width = 0;
		this->height = 0;
	}
};

typedef struct {
	Box box;
	float rot;
} OOBox;

struct Ray {
	Vec2D origin;
	Vec2D direction; // Should be normalized
};

struct OBB {
	Vec2D center;
	Vec2D halfExtents; // Half width and height
	float rotation;    // In radians
};

enum NodeType {
	NODE,
	NODE2D,
	SURFACE,
	SPRITE,
	PHYS,
};

struct StepParams {

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
	virtual void Render(RenderParams* p);
	virtual void RenderGraph(RenderParams *p);
	Node(Node* parent);
	Node();
};

void dumpError();

class Node2D : public Node {
public:
	// rotation
	float rads;

	Vec2D localPos;
	Vec2D globalPos;

	long int CurTime();
	int CurTime_Seconds();

	Vec2D RotatePoint(Vec2D p, float a);
	Transform GetGlobalPositionTransform();

	float GetLocalRotation();
	float GetGlobalRotation();

	void SetLocalPos(Vec2D p);
	float ToScreen(
		double sim_x, double sim_y,
		double* screen_x, double* screen_y,
		CameraData camera, int screen_width, int screen_height);
	
	void Render(RenderParams* p);
	Vec2D GetLocalPos();
	Node2D();
};

/// <summary>
/// Impliments solid checking for the node object
/// </summary>
class Solid2D : public Node2D {
protected:
	Box bounds;
	virtual OBB GetOOBounds();
	virtual Box GetAABounds();
	void SetBounds(int width, int height);
	bool ShouldCollide(Solid2D * caller);
	bool RayIntersectsOBB(const Ray& ray, const OBB& obb, float& tEntry, float& tExit);
	void RaycastSearch(std::string filter, Vec2D origin, Vec2D dir);
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
	//void Render(RenderParams* p);
};

class Sprite : public Surface {
	//int width;
	//int height;
	SDL_Surface* surf;
	SDL_Texture* texture;
	std::string filename;
public:
	void constructTexture(SDL_Renderer *p);
	Box GetSpriteSize();
	void SamplePoints(std::vector<Vec2D> *p); // RRGGBBAA
	void Step(double dt, Node* parent);
	void Render(RenderParams* p);
	static Sprite* FromSprite(Sprite* sprite, int width, int height);
	static Sprite* FromDisk(std::string filename);
};

class Phys2D : public Solid2D {
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