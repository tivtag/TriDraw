#pragma once

#include <string>
#include "AssetLoader.hpp"

class Image
{
public:
   unsigned int width() const { return _width; }
   unsigned int height() const { return _height; }
   unsigned char* data() const { return _data; }

public:
   Image();
   ~Image();
         
   bool loadFromAsset(const std::string& assetName, AssetLoader&);

private:
   unsigned char* _data;
   unsigned int _width, _height;
};
