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
#include <functional>
//#define CLASSNAME(x) const char* ClassName() { return #x; }
//#define INIT_NODE(CLASS, BASE) CLASS(): BASE() { this->nodetable.push_back(#CLASS); } const char* ClassName() { return #CLASS; }

#define DECLARE_NODE(CLASS, BASE) \
	class CLASS: public BASE { \
	using super = BASE; \
	friend Node; \
	protected: \
	CLASS(): BASE() { this->nodetable.push_back(#CLASS); } \
	public:	\
	static const char * GetClassName() { return #CLASS; } \
	const char* ClassName() { return #CLASS; }

#define HOOK(RET, name, ARGS, call) \
    RET name ARGS override { \
		/*printf("called  %s", #name);*/ \
        super::name call; \
        _##name call; \
    } \
	RET _##name ARGS

#define END_DECLARE_NODE(CLASS) };

#define DECLARE_BASE_NODE(CLASS) \
	class CLASS { \
	protected: \
	CLASS() { this->nodetable.push_back(#CLASS); } \
	public: \
	static const char* GetClassName() { return #CLASS; } \
	virtual const char* ClassName() { return #CLASS; }


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
	float width;
	float height;
	void Zero() {
		this->pos.x = 0;
		this->pos.y = 0;
		this->width = 0;
		this->height = 0;
	}
};

struct RenderCtx {
	SDL_Renderer* g;
	glm::mat4 M_proj;
	glm::mat4 M_view;
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

class Node; // forward declare clas s name so we can make the ref type before the class exists
typedef Node* NodeRef;

DECLARE_BASE_NODE(Node)
	Node* root_Cached;
	std::string Id;
	int uuid = GetUID();
	int index = 0;
	
	std::vector<std::string>hooks;

	//std::vector<std::unique_ptr<Node*>> children;
	std::vector<Node*> children;

	bool isDead = false;
	
	NodeType type = NodeType::NODE;
	Node* parent = NULL;

	std::vector<std::string> nodetable;

	bool _TraverseForwards(Node* node, const std::function<bool(Node*)>& cb);
	bool TraverseForwards(const std::function<bool(Node*)>& cb);
	void TraverseBackwards(const std::function<void(Node* node)>);

	int GetUID();
	void SetId(std::string Id);

	void SetParent(Node* parent);
	void AddChild(Node* node);
	void RemoveChild(NodeRef ref);
	void RemoveSelf();

	//void OnCreating();
	//void OnParentChanged();
	virtual void OnCreated() {}
	virtual void OnDestroying() {}

	bool InheritsFrom(const std::string&);

	// called when added as child to another node
	virtual void OnAddedToTree(Node* caller) {}
	
	/// <summary>
	/// Search for Node in graph if it starts with "/" then search from the Root else search from this nodes children
	/// Eg: /PlayHud/Overlay == root -> get child PlayHud -> get Child Overlay -> return Overlay
	/// </summary>
	/// <param name="query"></param>
	/// <returns></returns>
	/// 
	Node* GetNode(std::string query);
	Node* GetRoot();
	Node* GetChild(std::string Id);
	Node* GetParent(std::string Id);
	virtual void Dispose();
	virtual void DoEvent(input_event_args* args);
	virtual void OnStep(double dt, Node* parent);
	
	//template<class T, class... Args>
	//static std::unique_ptr<T> CreateNode(Args&&... args) {
	//	auto p = std::make_unique<T>(std::forward<Args>(args)...);
	//	p->OnCreated();
	//	return p;
	//}

	template<class T, class... Args>
	static T* New(Args&&... args) {
		T* p = new T();
		p->OnCreated();
		return p;
	}

	~Node();
	Node(Node* parent);
};

void dumpError();

DECLARE_NODE(Node2D, Node)
	float rot;
	Vec2D scale = { 1.0f, 1.0f };

	enum NodeFlags {
		NONE = 0,
		SKIP_TRANSFORM = 1 << 0
	} flags;

	Vec2D localPos = { 0, 0 };
	//Vec2D globalPos = { 0, 0 };
	Vec2D pivot = { 0, 0 };

	static Vec2D lerp(const Vec2D& a, const Vec2D& b, float t);
	long int CurTime();
	int CurTime_Seconds();

	glm::vec2 GetPositionFromMatrix(const glm::mat4& matrix);

	glm::mat4 GetLocalTransform(); 
	glm::mat4 GetWorldTransform();
	glm::vec2 GetWorldPos();
	glm::vec2 WorldToLocal(const glm::vec2& worldPos, Node2D* parent);

	Vec2D RotatePoint(Vec2D p, float a);
	float GetLocalRotation();
	float GetGlobalRotation();
	void SetLocalPos(Vec2D p);
	float ToScreen(
		double sim_x, double sim_y,
		double* screen_x, double* screen_y,
		CameraData camera, int screen_width, int screen_height);
	bool ShouldRender();
	Vec2D GetLocalPos();

END_DECLARE_NODE(Node2D)

/// <summary>
/// Impliments solid checking for the node object
/// </summary>a
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

DECLARE_NODE(Renderable, Node2D)
protected:
	int texwidth;
	int texheight;
	int width; 
	int height;
	glm::vec2 rLocal;
	glm::vec4 p0;
public:
	bool RenderTrap = false;
	//void GetTransformMatrix();
	glm::mat4 GetGlobalPositionTransformWithCamera();
	virtual void OnRender(RenderCtx* p) {};
	virtual void OnRenderEnd(RenderCtx* p) {};
	// render entry DO NOT OVERRIDE
	// if you want to add your own render logic, HOOK or overwrite OnRender
	void Render(RenderCtx* p);
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
	HOOK(void, OnStep, (double dt, Node* parent), (dt, parent));
};

#endif