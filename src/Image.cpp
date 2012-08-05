#include "Image.hpp"
#include "Log.hpp"

#include "stb_image.hpp"

Image::Image()
   : _data(0), _width(0), _height(0)
{
}

Image::~Image()
{
   if(_data)
   {
      stbi_image_free(static_cast<stbi_uc*>(_data));
      _data = 0;
   }
}

bool Image::loadFromAsset(const std::string& assetName, AssetLoader& assetLoader)
{
   const std::vector<unsigned char> buffer = assetLoader.read(assetName);
   const std::size_t size = buffer.size();
   
   if(size > 0)
   {
      int width(0), height(0), comp(0);
      stbi_uc* image_data = stbi_load_from_memory(static_cast<const stbi_uc*>(&buffer[0]), size, &width, &height, &comp, 4);
  
      if(image_data)
      {
         this->_width = width;
         this->_height = height;
         this->_data = static_cast<unsigned char*>(image_data);

         LOGI("Loaded Image from asset w=%i h=%i", width, height);
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
}
