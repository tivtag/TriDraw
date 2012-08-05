#pragma once

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "Log.hpp"

static void printGLString(const char* name, const GLenum s) 
{
   const char *v = (const char *)glGetString(s);
   LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* operation)
{
   GLenum error = glGetError();

   for(; error; error = glGetError())
   {
      LOGI("after %s() glError (0x%x)\n", operation, error);
   }
}
