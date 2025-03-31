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
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/transform2.hpp>  // For 2D transformations

//#define CLASSNAME(x) const char* ClassName() { return #x; }
//#define INIT_NODE(CLASS, BASE) CLASS(): BASE() { this->nodetable.push_back(#CLASS); } const char* ClassName() { return #CLASS; }
#define DECLARE_NODE(CLASS, BASE) \
	class CLASS: public BASE { \
	public: \
	CLASS(): BASE() { this->nodetable.push_back(#CLASS); } \
	static const char* GetTypeName() { return #CLASS; } \
	const char* ClassName() { return #CLASS; }
#define END_DECLARE_NODE(CLASS) };

#define DECLARE_BASE_NODE(CLASS) \
	class CLASS { \
	public: \
	CLASS() { this->nodetable.push_back(#CLASS); } \
	static const char* GetTypeName() { return #CLASS; } \
	const char* ClassName() { return #CLASS; }

int InitLibs();

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

// Normalize between -π and π
float NormalizeRotation(float rads);

// Ensure the angle is between 0 and 2*π (0 to 360 degrees)
float ClampRotation(float rads);
#define Vec2D glm::vec2

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

struct RenderParams {
	SDL_Renderer* g;
	SDL_Texture* layer;
	SDL_Texture* Layer(Box* bhint);
};

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

class Node;
typedef Node* NodeRef;

DECLARE_BASE_NODE(Node) //{
public:
	//const char* ClassName() { throw "undefined ClassName"; }
	
	Node* root_Cached;
	int uuid = GetUID();
	std::string Id;
	//std::string Type;
	bool isDead = false;
	NodeType type = NodeType::NODE;
	Node* parent = NULL;
	int index = 0;
	
	std::vector<std::string> nodetable;

	//std::vector<std::unique_ptr<Node*>> children;
	std::vector<Node*> children;
	int GetUID();
	void SetId(std::string Id);
	void AddChild(Node* node);
	void RemoveChild(NodeRef ref);
	void RemoveSelf();
	void SetParent(Node* parent);

	bool InheritsFrom(const std::string&);

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
	
	//virtual void Render(RenderParams* p);
	//virtual void RenderGraph(RenderParams *p);
	~Node();
	Node(Node* parent) : Node() {
		//gctable.push_back(this);
	};
};

void dumpError();

DECLARE_NODE(Node2D, Node)
	float rads;

	glm::mat4 localTransform;
	glm::mat4 globalTransform;
	Vec2D scale = { 1.0f, 1.0f };

	enum NodeFlags {
		NONE = 0,
		SKIP_TRANSFORM = 1 << 0
	} flags;

	Vec2D localPos;
	Vec2D globalPos;

	static Vec2D lerp(const Vec2D& a, const Vec2D& b, float t);
	long int CurTime();
	int CurTime_Seconds();
	glm::vec2 GetPositionFromMatrix(const glm::mat4& matrix);

	void GetLocalTransform(); 

	glm::mat4 GetGlobalPositionTransform();

	Vec2D RotatePoint(Vec2D p, float a);

	//void UpdateGlobalTransform();
	float GetLocalRotation();
	float GetGlobalRotation();

	void SetLocalPos(Vec2D p);
	float ToScreen(
		double sim_x, double sim_y,
		double* screen_x, double* screen_y,
		CameraData camera, int screen_width, int screen_height);

	bool ShouldRender();
	//void Render(RenderParams* p);
	Vec2D GetLocalPos();

END_DECLARE_NODE(Node2D)

/// <summary>
/// Impliments solid checking for the node object
/// </summary>
DECLARE_NODE(Solid2D, Node2D) 
protected:
	Box bounds;
	virtual OBB GetOOBounds();
	virtual Box GetAABounds();
	bool ShouldRender();
	void SetBounds(int width, int height);
	bool ShouldCollide(Solid2D * caller);
	bool RayIntersectsOBB(const Ray& ray, const OBB& obb, float& tEntry, float& tExit);
	void RaycastSearch(std::string filter, Vec2D origin, Vec2D dir);
};

class Node2D_Test : public Node2D {
public:
	void Step(double dt, Node* parent);
};

DECLARE_NODE(Renderable, Node2D)
protected:
	int texwidth;
	int texheight;
	int width; 
	int height;
	
public:
	//void GetTransformMatrix();
	glm::mat4 GetGlobalPositionTransformWithCamera();
	void Step(double dt, Node* parent);
	virtual void OnRender(RenderParams* p);
	virtual void Render(RenderParams* p) final;
};

DECLARE_NODE(Sprite, Renderable)
	//int width;
	//int height;
	SDL_Surface* surf;
	SDL_Texture* texture;
	std::string filename;
public:
	Sprite* CopyRegion(Box p);
	void constructTexture(SDL_Renderer *p);
	Box GetSpriteSize();
	void SamplePoints(std::vector<Vec2D> *p); // RRGGBBAA
	void Step(double dt, Node* parent);

	void OnRender(RenderParams* p);
	void UpdateTextureInfo();
	void Dispose();
	static Sprite* FromDisk(std::string filename);
};

class AnimatedSprite : public Sprite {
	std::map<std::string, Sprite*> animationTable;
};

DECLARE_NODE(Phys2D, Node2D)
public:
	enum ProcessFlags {
		DO_GRAVITY = 1 << 0,
		DO_FRICTION = 1 << 1,  // 0010
		DO_COLLISION = 1 << 2,  // 0100
	};
	Solid2D* collider;
	
	double friction = 10;
	Vec2D velocity;
	Vec2D acceleration;
	
	const float G = 10000.0; //6.674;
	int physFlags = 0;
	// compute gravity
	void do_gravity(double dt);
	// compute friction
	void do_friction(double dt);
	void Step(double dt, Node* parent);
};

#endif