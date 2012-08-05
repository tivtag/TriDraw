#pragma once

#include <string>

#include "OpenGL.hpp"
#include "AssetLoader.hpp"
#include "Image.hpp"

class Texture
{
public:
   Texture();
   ~Texture();

   GLuint id() const { return textureId; }

   bool loadFromAsset(const std::string& assetName, AssetLoader&);
   bool loadFromImage(const Image&);

   void bind();

private:
   GLuint textureId;
};
