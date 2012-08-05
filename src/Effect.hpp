#pragma once

#include "OpenGL.hpp"

GLuint loadShader(const GLenum shaderType, const char* sourceCode);
GLuint createProgram(const char* vertexShaderCode, const char* fragmentShaderCode);
