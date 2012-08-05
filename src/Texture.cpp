#include "Texture.hpp"
#include "Image.hpp"
#include "Log.hpp"

Texture::Texture()
   : textureId(0)
{
}

Texture::~Texture()
{
   if(textureId)
   {
      glDeleteTextures(1, &textureId);
      checkGlError("glDeleteTextures");
      textureId = 0;
   }
}

bool Texture::loadFromAsset(const std::string& assetName, AssetLoader& assetLoader)
{
   bool loaded(false);

   Image image;
   if(image.loadFromAsset(assetName, assetLoader))
   {
      loaded = loadFromImage(image);
   }

   return loaded;
}

bool Texture::loadFromImage(const Image& image)
{
   glGenTextures(1, &textureId);
   checkGlError("glGenTextures");

   if(!textureId)
   {
      LOGE("Could not generate texture resource");  
      return false;
   }

   glBindTexture(GL_TEXTURE_2D, textureId);
   checkGlError("glBindTexture");

   glTexImage2D( 
      GL_TEXTURE_2D,    // target
      0,                // level
      GL_RGBA,          // internal format
      image.width(),    // width
      image.height(),   // height
      0,                // border  
      GL_RGBA,          // data format
      GL_UNSIGNED_BYTE, // data type
      image.data()      // data
   );
   checkGlError("glTexImage2D");

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   checkGlError("glTexParameteri GL_TEXTURE_MIN_FILTER");
      
   // non-power of two tetures *must* have those set
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   checkGlError("glTexParameteri GL_TEXTURE_WRAP_S ");
             
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   checkGlError("glTexParameteri GL_TEXTURE_WRAP_T");

   LOGI("Loaded Texture id=%i", textureId);
   glBindTexture(GL_TEXTURE_2D, 0);
   return true;
}

void Texture::bind()
{   
   glBindTexture(GL_TEXTURE_2D, textureId);
   checkGlError("glBindTexture");
}
