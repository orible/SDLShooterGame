// Game1.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

typedef struct {
	double x, y;
} Vec2D;

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

Camera camera;
Screen screen;
long int uuid = 0;

int GetUID() {
	return uuid++;
}

class Node;
std::vector<Node*> gctable;
class Node {
public:
	int uuid;
	std::string Id;
	bool isDead = false;
	NodeType type = NodeType::NODE;
	Node* parent;
	std::vector<Node*> children;
	void SetId(std::string Id) {
		this->Id = Id;
	}
	/// <summary>
	/// Search for Node in graph if it starts with "/" then search from the Root else search from this nodes children
	/// Eg: /PlayHud/Overlay == root -> get child PlayHud -> get Child Overlay -> return Overlay
	/// </summary>
	/// <param name="query"></param>
	/// <returns></returns>
	Node* GetNode(std::string query) {
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
	Node* GetRoot() {
		Node* node = this;
		while (node->parent != NULL) {
			node = node->parent;
		}
		return node;
	}
	Node* GetChild(std::string Id) {
		if (this->children.empty()) return NULL;
		for (int i = 0; i < this->children.size(); i++) {
			if (this->children[i]->Id == Id) {
				return this->children[i];
			}
		}
		return NULL;
	}
	Node* GetParent(std::string Id) {
		if (parent == NULL) {
			return NULL;
		}
		if (parent->Id == Id) {
			return parent;
		}
		parent->GetParent(Id);
	}
	virtual void Dispose() {
		if (this->isDead) return;
		for (int i = 0; i < this->children.size(); i++) {
			this->children[i]->Dispose();
		}
		this->isDead = true;
	}
	virtual void DoEvent(input_event_args *args) {
		if (this->isDead) return;
		for (int i = 0; i < this->children.size(); i++) {
			this->children[i]->DoEvent(args);
		}
	}
	void SetParent(Node* parent) {
		if (this->isDead) return;
		this->parent = parent;
	}
	virtual void Step(double dt, Node * parent) {
		if (this->isDead) return;
		for (int i = 0; i < this->children.size(); i++) {
			this->children[i]->Step(dt, this);
		}
	}
	virtual void Render(SDL_Renderer* g) {
		if (this->isDead) return;
		for (int i = 0; i < this->children.size(); i++) {
			this->children[i]->Render(g);
		}
	}
	void AddChild(Node* node) {
		if (this->isDead) return;
		node->parent = this;
		this->children.push_back(node);
	}
	Node(Node* parent) {
		this->uuid = GetUID();
		this->parent = parent;
		gctable.push_back(this);
	}
	Node() {
		this->uuid = GetUID();
		this->parent = NULL;
		gctable.push_back(this);
	}
};

void dumpError() {
	printf("SDL_GetError: %s\n", SDL_GetError());
	//SDL_DestroyWindow(window);
	SDL_Quit();
}

class Node2D : public Node {
public:
	float rads;
	Vec2D pos;
	long int CurTime() {
		return SDL_GetTicks();
	}
	Vec2D RotatePoint(Vec2D p, float a) {
		float cosa = cos(a);
		float sina = sin(a);
		return {
			p.x * cosa - p.y * sina,
			p.x * sina + p.y * cosa,
		};
	}
	virtual OOBox GetOOBounds() {
		return OOBox{ GetAABounds(), this->rads };
	}
	virtual Box GetAABounds() {
		return { pos.x, pos.y, 0, 0 };
	}

	float ToScreen(
		double sim_x, double sim_y,
		double* screen_x, double* screen_y,
		Camera camera, int screen_width, int screen_height) {
		*screen_x = (sim_x - camera.x) / camera.scale + screen_width / 2;
		*screen_y = (sim_y - camera.y) / camera.scale + screen_height / 2;
		return 0;
	}
	void RaycastSearch(std::string filter, Vec2D origin, Vec2D dir) {
		//void Box1 = GetAABounds();
		//(Node2D*)nodeRoot;
	}
	Point GetTransform() {
		Point point;
		ToScreen(pos.x, pos.y, &point.x, &point.y, camera, screen.width, screen.height);
	}
};

class Surface : public Node2D {
public:
	void Step(double dt, Node* parent) {
		Node2D::Step(dt, this);
	}
	void Render(SDL_Renderer* g) {
		SDL_Rect rect;
		
		SDL_RenderDrawRect(g, &rect);
		Node2D::Render(g);
	}
};

class TextBox : public Surface {
	TTF_Font* font = TTF_OpenFont("./assets/fonts/Broken Console Bold.ttf", 24);
	SDL_Texture* tex;
	bool dirty = true;
	std::string text;
	int fwidth, fheight;
public:
	void SetTextF(const char* fmt, ...) {
		va_list argp;
		va_start(argp, fmt);

		char* buf = new char[255];
		int ret = vsnprintf(buf, 255, fmt, argp);
		std::string text(buf, ret);
		SetText(text);
		va_end(argp);
	}
	void SetText(std::string text) {
		this->text = text;
		this->dirty = true;
	}
	bool IsDirty() {
		return this->dirty;
	}
	TextBox(int x, int y, int width, int height) : Surface() {
		this->pos.x = x;
		this->pos.y = y;
	}
	void Render(SDL_Renderer* g) {
		if (this->dirty) {
			if (this->tex != NULL) {
				SDL_DestroyTexture(this->tex);
				this->tex = NULL;
			}
			SDL_Color textColor = { 255, 255, 255, 0 };
			SDL_Color bgColor = { 10, 10, 10, 0 };
			SDL_Surface* surf = TTF_RenderText_Shaded_Wrapped(this->font, this->text.c_str(), textColor, bgColor, 0);
			if (surf == NULL) {
				Surface::Render(g);
				return;
			}
			fwidth = surf->w;
			fheight = surf->h;
			tex = SDL_CreateTextureFromSurface(g, surf);
			SDL_FreeSurface(surf);
			this->dirty = false;
		}
		SDL_Rect rect{ pos.x, pos.y, fwidth, fheight };
		SDL_RenderCopy(g, tex, NULL, &rect);
		Surface::Render(g);
	}
};

class HUD : public Surface {
public:
	TextBox* debugInfo = new TextBox(0, 0, 100, 100);
	TextBox* inputInfo = new TextBox(250, 0, 100, 100);
	void UpdatePerformance() {
		debugInfo->SetText("Hello!");
	}

	void Render(SDL_Renderer* g) {
		debugInfo->Render(g);
		inputInfo->Render(g);
		Surface::Render(g);
	}
};

class Sprite : public Node2D {
	int width;
	int height;
	SDL_Surface* surf;
	SDL_Texture* texture;
	std::string filename;
public:
	void constructTexture(SDL_Renderer* g) {
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
	void Step(double dt, Node* parent) {
		this->rads = cos(CurTime()/1000.f);
		//printf("%d\n", this->rads);
		Node2D::Step(dt, parent);
	}
	void Render(SDL_Renderer* g) {
		// assert texture is constructed all lazy like probably boils to EQ idk, TODO: inspect the assemblr
		constructTexture(g);

		double x = ((Node2D*)this->parent)->pos.x + this->pos.x;
		double y = ((Node2D*)this->parent)->pos.y + this->pos.y;

		// compute transient fields
		SDL_Rect dstrect = { x, y, width, height};
		SDL_RenderCopyEx(g, this->texture, NULL, &dstrect, this->rads * (100.0f / M_PI), NULL, SDL_FLIP_NONE);
		Node2D::Render(g);
	}
	static Sprite* FromDisk(std::string filename) {
		Sprite * sp = new Sprite();
		sp->filename = filename;
		sp->surf = SDL_LoadBMP(("./assets/" + filename).c_str());
		return sp;
	}
};

class Phys2D : public Node2D {
public:
	double friction;
	Vec2D velocity;
	Vec2D acceleration;
	const float G = 10000;//6.674;

	// compute gravity
	void do_gravity(double dt) {
		Vec2D v1 = { 0, -100 };
		Vec2D v2 = this->pos;
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
	void do_friction(double dt) {
		// SQRT(v.x^2 + v.y^2)
		double speed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
		if (speed > 0) {
			double decel = friction * 9.8 * dt;
			double new_speed = fmax(0, speed - decel);
			velocity.x *= new_speed / speed;
			velocity.y *= new_speed / speed;
		}
	}

	void Step(double dt, Node * parent) {
		double mag = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
		if (mag > 0) {
			// break
			int a = 0;
		}

		do_gravity(dt);

		do_friction(dt);
		
		pos.x += velocity.x * dt;
		pos.y += velocity.y * dt;
		((HUD*)this->GetNode("/PlayHUD"))->debugInfo->SetTextF(
			"Pos: %.1f x %.1f\nVel: %.1f x %.1f\nMag: %f", 
			pos.x, pos.y, 
			velocity.x, velocity.y, 
			mag);
		//velocity.x += acceleration.x * dt;
		//velocity.y += acceleration.y * dt;
		Node2D::Step(dt, this);
	}
};

class Actor : public Phys2D {
	Sprite* sprite;
	Vec2D dir = { 0, 0 };

public:
	void computeInputVectors(SDL_Event event) {
		dir = {0, 0};
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_w:
				dir.y -= 1.0f;  // Move up
				break;
			case SDLK_a:
				dir.x -= 1.0f;  // Move left
				break;
			case SDLK_s:
				dir.y += 1.0f;  // Move down
				break;
			case SDLK_d:
				dir.x += 1.0f;  // Move right
				break;
			}
			break;
		case SDL_KEYUP:
			/*switch (event.key.keysym.sym) {
			case SDLK_w:
				dir.y += 1.0f;  // Stop moving up
				break;
			case SDLK_a:
				dir.x += 1.0f;  // Stop moving left
				break;
			case SDLK_s:
				dir.y -= 1.0f;  // Stop moving down
				break;
			case SDLK_d:
				dir.x -= 1.0f;  // Stop moving right
				break;
			}
			break;/**/
		default:
			return;
		}
		//SDL_Text
		// Normalize the movement vector (to prevent faster diagonal movement)
		float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
		if (length > 0.0f) {
			dir.x /= length;
			dir.y /= length;
		}
		printf("%f\n", length);
		((HUD*)this->GetNode("/PlayHUD"))->inputInfo->SetTextF("W[] S[] A[] D[] SPACE[] %f", length);
	}
	
	void DoEvent(input_event_args* args) {
		computeInputVectors(args->ev);
		
		const float impulse = 10.0f;
		float mag = sqrtf(dir.x * dir.x + dir.y * dir.y);
		
		Vec2D force = dir;
		if (mag > 0.0001f) {
			force.x /= mag;
			force.y /= mag;
			// add force
			this->velocity.x += force.x * impulse;
			this->velocity.y += force.y * impulse;
		}

		if (mag > 0) {
			// spin
			this->rads + 1 * mag;
		}
	}

	Actor(): Phys2D() {
		this->sprite = Sprite::FromDisk("./player.bmp");
		// this will set the sprite parent and hook it as a child that is auto executed on step, render etc
		this->AddChild(sprite);
	}
	//Sprite * sprite = new Sprite();
};

/// <summary>
// World root, must be a Node2D as we need raycast search functions and other helpers
/// </summary>
Node2D * nodeRoot = NULL;

int main(int argc, char* args[])
{
	SDL_Surface* winSurface = NULL;
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	screen.width = 1000;
	screen.height = 1000;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return -1;
	}

	window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen.width, screen.height, SDL_WINDOW_SHOWN);

	if (!window) {
		return -1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	TTF_Init();

	nodeRoot = new Surface();
	nodeRoot->SetId("Root");

	Node* hud = new HUD();
	hud->SetId("PlayHUD");
	nodeRoot->AddChild(hud);
	nodeRoot->GetNode("/PlayHUD");

	long int prev_ticks = SDL_GetPerformanceCounter();
	long int prev_poll_ticks = SDL_GetPerformanceCounter();

	double dt = 0;
	double poll_dt = 0;
	int freq = SDL_GetPerformanceFrequency();
	bool isRunning = true;

	eventbuf * evbuf = new eventbuf;

	nodeRoot->AddChild(new Actor());

	while (isRunning) {
		long int ticks = SDL_GetPerformanceCounter();
		dt = (ticks - prev_ticks) / (double)freq;
		poll_dt = (ticks - prev_poll_ticks) / (double)freq;
		
		// new struct every loop as copied fields is not determinable
		SDL_Event ivent;
		if (SDL_PollEvent(&ivent) == 1) {
			if (ivent.type == SDL_QUIT) {
				isRunning = false;
			}

			//evbuf->push_back(ivent);
			// execute input events
			nodeRoot->DoEvent(new input_event_args{ evbuf, ivent });
		}
		poll_dt = prev_ticks;

		// execute physics
		nodeRoot->Step(dt, NULL);

		// blank frame
		//SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 255, 255, 255));

		SDL_RenderClear(renderer);

		// render back to front
		nodeRoot->Render(renderer);
		
		SDL_RenderPresent(renderer);
		//SDL_UpdateWindowSurface(window);

		//printf("tick => %f\n", dt);

		prev_ticks = ticks;
		SDL_Delay(16);
	}

	printf("END\n");
	system("pause");

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}