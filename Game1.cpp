// Game1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <filesystem>
#include <iostream>
#include <vector>
#include <SDL.h>

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

// Transform Simulation Coordinates to Screen Coordinates:
int transform_to_screen(
	double sim_x, double sim_y,
	double* screen_x, double* screen_y,
	Camera camera, int screen_width, int screen_height) {
	*screen_x = (sim_x - camera.x) / camera.scale + screen_width / 2;
	*screen_y = (sim_y - camera.y) / camera.scale + screen_height / 2;
	return 0;
}

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
	bool isDead = false;
	Node* parent;
	std::vector<Node*>children;
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
	float rot;
	Vec2D pos;
	Point GetTransform() {
		Point point;
		transform_to_screen(pos.x, pos.y, &point.x, &point.y, camera, screen.width, screen.height);
	}

	//Node2D(Node* parent) : Node(parent) {}
};

class Surface : public Node2D {
	void Step(double dt, Node* parent) {
		Node::Step(dt, this);
	}
	void Render(SDL_Renderer* g) {
		SDL_Rect rect;
		
		SDL_RenderDrawRect(g, &rect);
		Node2D::Render(g);
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

	void Render(SDL_Renderer* g) {
		// assert texture is constructed all lazy like probably boils to EQ idk, TODO: inspect the assemblr
		constructTexture(g);

		double x = ((Node2D*)this->parent)->pos.x + this->pos.x;
		double y = ((Node2D*)this->parent)->pos.y + this->pos.y;

		// compute transient fields

		SDL_Rect dstrect = { x, y, width, height};
		SDL_RenderCopyEx(g, this->texture, NULL, &dstrect, this->rot, NULL, SDL_FLIP_NONE);
		Node2D::Render(g);
	}
	static Sprite* FromDisk(std::string filename) {
		Sprite * sp = new Sprite();
		sp->filename = filename;
		
		//std::filesystem::current_path();
		sp->surf = SDL_LoadBMP(("./assets/" + filename).c_str());
		return sp;
	}
	//Sprite(Node*parent) : Node2D(parent) {}
};

class Phys2D : public Node2D {
public:
	double friction;
	Vec2D velocity;
	Vec2D acceleration;

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
		pos.x += velocity.x * dt;
		pos.y += velocity.y * dt;

		do_friction(dt);

		velocity.x += acceleration.x * dt;
		velocity.y += acceleration.y * dt;
		Node2D::Step(dt, this);
	}
};

class Actor : public Phys2D {
	Sprite* sprite;
	Vec2D dir = { 0, 0 };	
public:
	void computeInputVectors(SDL_Event event) {
		dir = { 0, 0 };
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

		/*case SDL_KEYUP:
			switch (event.key.keysym.sym) {
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
			break;*/
		}
		SDL_Text
		// Normalize the movement vector (to prevent faster diagonal movement)
		float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
		if (length > 0.0f) {
			dir.x /= length;
			dir.y /= length;
		}
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
			this->rot + 1 * mag;
		}
	}

	Actor(): Phys2D() {
		this->sprite = Sprite::FromDisk("./player.bmp");
		// this will set the sprite parent and hook it as a child that is auto executed on step, render etc
		this->AddChild(sprite);
	}
	//Sprite * sprite = new Sprite();
};

Node* nodeRoot = NULL;

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

	//winSurface = SDL_GetWindowSurface(window);
	//if (!winSurface) {
	//	return -1;
	//}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	//renderer = SDL_GetRenderer(window);


	nodeRoot = new Surface();

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
		SDL_Event ivent;
		//SDL_PumpEvents();

		//if (poll_dt > .8) {
			if (SDL_PollEvent(&ivent) == 1) {
				if (ivent.type == SDL_QUIT) {
					isRunning = false;
				}

				//evbuf->push_back(ivent);
				// execute input events
				nodeRoot->DoEvent(new input_event_args{ evbuf, ivent });
			}
			poll_dt = prev_ticks;
		//}

		// execute physics
		nodeRoot->Step(dt, NULL);

		// blank frame
		//SDL_FillRect(winSurface, NULL, SDL_MapRGB(winSurface->format, 255, 255, 255));

		SDL_RenderClear(renderer);

		// render back to front
		nodeRoot->Render(renderer);
		
		SDL_RenderPresent(renderer);
		//SDL_UpdateWindowSurface(window);

		printf("tick => %f\n", dt);

		prev_ticks = ticks;
		SDL_Delay(16);
	}

	printf("END\n");
	system("pause");

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}