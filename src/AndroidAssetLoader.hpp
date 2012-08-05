#pragma once

#include "AssetLoader.hpp"

struct AAssetManager;

class AndroidAssetLoader : public AssetLoader
{
public:
   AndroidAssetLoader(AAssetManager* _assetManager) 
      : assetManager(_assetManager)
   {
   }

   virtual std::vector<unsigned char> read(const std::string& assetName);

private:
   AAssetManager* assetManager;
};
