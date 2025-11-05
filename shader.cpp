#include "shader.h"
#include "engine.h"

//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <SDL.h>

#include <SDL_opengl.h>
#include <gl\glu.h>
#include <stdio.h>
#include <unordered_map>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> RenderableGPU::shaderCache{};

ShaderProgram* RenderableGPU::findCache(const std::string& key)
{
    auto find = shaderCache.find(key);
    if (find == shaderCache.end()) {
        return nullptr;
    }

    return find->second.get();
}

void RenderableGPU::_OnCreated() {

}

GLint RenderableGPU::checkShader(GLuint sh)
{
    GLint ok = 0; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        //std::string log(len, '\0'); glGetShaderInfoLog(sh, len, nullptr, log.data());
        return -1;
    }
    return ok;
}

GLint RenderableGPU::checkProgram(GLuint prog)
{
    GLint ok = 0; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        
        return -1;
    }
    return ok;
}

GLuint RenderableGPU::compile(GLenum type, const std::string& src)
{
    GLuint sh = glCreateShader(type);
    const char* csrc = src.c_str();
    glShaderSource(sh, 1, &csrc, nullptr);
    glCompileShader(sh);
    if (!checkShader(sh)) {
        return -1;
    }
    return sh;
}

std::string RenderableGPU::loadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Failed to open file: " + path);
    std::ostringstream ss; ss << f.rdbuf();
    return ss.str();
}

ShaderProgram* RenderableGPU::GetOrLoad(
    const std::string& key, 
    ShaderBlob &blob)
{
    std::string vsrc = loadFile(blob.vertPath);
    std::string fsrc = loadFile(blob.fragPath);

    GLuint vs = 0, fs = 0, prog = 0;
    
   vs = compile(GL_VERTEX_SHADER, vsrc);
   if (vs == -1) {
       return nullptr;
   }

   fs = compile(GL_FRAGMENT_SHADER, fsrc);
   if (fs == -1) {
       if (vs) glDeleteShader(vs);
       return nullptr;
    }

   prog = glCreateProgram();
   if (prog == 0) {
       if (vs) glDeleteShader(vs);
       if (fs) glDeleteShader(fs);
       return nullptr;
   }

   glAttachShader(prog, vs);
   glAttachShader(prog, fs);
   glLinkProgram(prog);
   if (checkProgram(prog) == -1) {
       if (vs) glDeleteShader(vs);
       if (fs) glDeleteShader(fs);
       if (prog) glDeleteProgram(prog);
   }


   glDetachShader(prog, vs);
   glDetachShader(prog, fs);
   glDeleteShader(vs);
   glDeleteShader(fs);

    auto sp = std::make_unique<ShaderProgram>();
    sp->Id = prog;
    ShaderProgram* out = sp.get();
    
    //cache_.emplace(key, std::move(sp));
    return out;
}

ShaderProgram::~ShaderProgram() { 
    if (Id) {
        glDeleteProgram(Id);
    }
}
