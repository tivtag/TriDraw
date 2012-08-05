#include "WindowsAssetLoader.hpp"

#include <iterator>
#include <fstream>

#include "windows.h"
#include "Log.hpp"

std::wstring GetStartUpPath()
{
   WCHAR result[MAX_PATH + 1] = L"";
   GetModuleFileNameW(NULL, result, MAX_PATH);

   const std::wstring path( result );
   const std::wstring::size_type pos = path.find_last_of( L"\\/" );
   return path.substr(0, pos);
}

WindowsAssetLoader::WindowsAssetLoader()
   : basePath(GetStartUpPath())
{
}

std::vector<unsigned char> WindowsAssetLoader::read(const std::string& assetName)
{
   const std::wstring filePath = basePath + L"\\assets\\" + std::wstring(assetName.begin(), assetName.end());
   std::ifstream file(filePath, std::ios_base::in | std::ios_base::binary);

   std::vector<unsigned char> fileContent;

   if( file.good() )
   {
      // Slow! Should be optimized.
      unsigned char ch = file.get(); 
      while(file.good())
      {
         fileContent.push_back(ch);
         ch = file.get();
      }
   }
   else
   {
      LOGE("File %s not found", filePath.c_str());
   }

   return fileContent;
}
