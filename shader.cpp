#include "shader.h"
#include "engine.h"

//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <SDL.h>

#include <SDL_opengl.h>
#include <gl\glu.h>
#include <stdio.h>
#include <string>

DECLARE_NODE(GPUShader, Renderable)
	void loadShader();

END_DECLARE_NODE();