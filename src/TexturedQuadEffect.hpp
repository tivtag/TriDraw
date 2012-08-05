#pragma once
#include "OpenGL.hpp"

class Texture;

class TexturedQuadEffect
{
public:
   bool flipY() const { return _flipY; }
   void flipY(const bool value) { _flipY = value; }

   TexturedQuadEffect();
   ~TexturedQuadEffect();

   bool load();
   void draw();
   
private:
   GLuint program;
   GLuint positionAttribute, uvAttribute;

   GLuint vbo, flipVbo; 
   bool _flipY;
};
