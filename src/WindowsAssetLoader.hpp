#pragma once

#include "AssetLoader.hpp"

class WindowsAssetLoader : public AssetLoader
{
public:
   WindowsAssetLoader();
   virtual std::vector<unsigned char> read(const std::string& assetName);

private:
   const std::wstring basePath;
};
