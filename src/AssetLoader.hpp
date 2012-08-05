#pragma once

#include <vector>
#include <string>

class AssetLoader
{
protected:
   AssetLoader() {}

public:
   virtual ~AssetLoader() { }
   virtual std::vector<unsigned char> read(const std::string& assetName) = 0;
};
