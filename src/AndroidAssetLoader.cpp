#include "AndroidAssetLoader.hpp"
#include "Log.hpp"

#include <android_native_app_glue.h>
#include <android/asset_manager.h>

std::vector<unsigned char> AndroidAssetLoader::read(const std::string& assetName)
{
   long size = -1;
   unsigned char* buffer = 0;

   // Load from APK
   AAsset* asset = AAssetManager_open(assetManager, assetName.c_str(), AASSET_MODE_UNKNOWN);

   if(asset)
   {
      size = AAsset_getLength(asset);
      LOGI("Loaded asset. Length=%i", size);

      buffer = new unsigned char[size];
      if(buffer)
      {
         AAsset_read(asset, buffer, size);
      }
      else
      {
         LOGE("Could not make asset buffer");
      }

      AAsset_close(asset);
   }
   
   if(buffer)
   {
      return std::vector<unsigned char>(buffer, buffer + size);
   }
   else
   {
      return std::vector<unsigned char>();
   }
}
