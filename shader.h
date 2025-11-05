#pragma once

#include "engine.h"
#include <gl\glew.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <iostream>
#include <vector>
#include <SDL_opengl_glext.h>
#include <unordered_map>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

struct ShaderBlob {
	std::string vertPath;
	std::string fragPath;
	ShaderBlob(const std::string vertPath, const std::string fragPath) {
		this->fragPath = fragPath;
		this->vertPath = vertPath;
	}
};

class ShaderProgram {
public:
	GLuint Id = 0;
	~ShaderProgram();
	//ShaderProgram(const ShaderProgram&) = delete;
	//ShaderProgram& operator=(const ShaderProgram&) = delete;
};

DECLARE_NODE(RenderableGPU, Renderable)
static std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> shaderCache;
	
	HOOK(void, OnCreated, (), ());

	static ShaderProgram* findCache(const std::string& key);
	static GLint checkShader(GLuint sh);
	static GLint checkProgram(GLuint prog);
	static GLuint compile(GLenum type, const std::string& src);
	static std::string loadFile(const std::string& path);
	//static ShaderProgram* GetOrLoad()
	static ShaderProgram* GetOrLoad(const std::string& key,
		ShaderBlob &blob);
END_DECLARE_NODE();