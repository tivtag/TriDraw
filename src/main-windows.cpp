#include <vector>
#include <windows.h>
#include <TCHAR.H>

#include "Log.hpp"
#include "OpenGL.hpp"
#include "Random.hpp"

#include "Simulation.hpp"
#include "WindowsAssetLoader.hpp"

#define WINDOW_CLASS _T("TriDrawClass")

const int ResolutionX = 540;
const int ResolutionY = 960;

Simulation* simulation;
AssetLoader* assetLoader = 0;
HWND hWnd;

GLuint viewportWidth = -1;
GLuint viewportHeight = -1;
float frameTime = 0.0f;

int animating = 1;
EGLDisplay display = 0;
EGLSurface surface = 0;
EGLContext context = 0;

void engine_draw_frame();
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch(message)
   {
      case WM_CLOSE:
         exit(0);
         break;

      case WM_SYSKEYDOWN:
      case WM_KEYDOWN:
         simulation->toggleOutputTexture();
         break;
   }

   return DefWindowProc(hWnd, message, wParam, lParam);
}

/**
 * Initialize an EGL context for the current display.
 */
static bool engine_init_display(HINSTANCE hInstance) 
{
   // initialize OpenGL ES and EGL
   LOGI("Begin engine_init_display");

   /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
   const EGLint attribs[] = {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, //important 
      EGL_BLUE_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_RED_SIZE, 8,
      EGL_NONE
   };
   const EGLint attrib_list [] = {
      EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
   };
    
   EGLint w, h, format;
   EGLint numConfigs;
   EGLConfig config;
   EGLSurface surface;
   EGLContext context;

   display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   eglInitialize(display, 0, 0);

   /* Here, the application chooses the configuration it desires. In this
   * sample, we have a very simplified selection process, where we pick
   * the first EGLConfig that matches our criteria */
   eglChooseConfig(display, attribs, &config, 1, &numConfigs);

   /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
   eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

   int x, y;
   RECT winRect;

   SetRect(&winRect, 0, 0, ResolutionX, ResolutionY);
   AdjustWindowRectEx(&winRect, WS_CAPTION|WS_SYSMENU, false, 0);

   x = 0 - winRect.left;
   winRect.left += x;
   winRect.right += x;

   y = 0 - winRect.top;
   winRect.top += y;
   winRect.bottom += y;

   if(true)
   {
      x = CW_USEDEFAULT;
      y = CW_USEDEFAULT;
   }
   else
   {
      x = winRect.left;
      y = winRect.top;
   }

   WNDCLASS wc;
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = (WNDPROC)WndProc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.hInstance     = hInstance;
   wc.hIcon         = 0;
   wc.hCursor       = 0;
   wc.lpszMenuName  = 0;
   wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
   wc.lpszClassName = WINDOW_CLASS;
   RegisterClass(&wc);

   hWnd = CreateWindow(
      WINDOW_CLASS, _T("tri-draw"),
      WS_VISIBLE|WS_CAPTION|WS_SYSMENU,
      x, y, winRect.right-winRect.left, winRect.bottom-winRect.top,
      NULL, NULL, hInstance, NULL
   );

   surface = eglCreateWindowSurface(display, config, hWnd, NULL); // ToDO
   if(!surface)
   {
      LOGE("Unable to eglCreateWindowSurface");
      return false;
   }

   context = eglCreateContext(display, config, NULL, attrib_list);
   if(!surface)
   {
      LOGE("Unable to eglCreateContext");
      return false;
   }

   if(eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
      LOGE("Unable to eglMakeCurrent");
      return false;
   }

   eglQuerySurface(display, surface, EGL_WIDTH, &w);
   eglQuerySurface(display, surface, EGL_HEIGHT, &h);
   viewportWidth = w / Constants::DownScaleFactor;
   viewportHeight = h / Constants::DownScaleFactor;
   
   // Initialize GL state.
   eglSwapInterval(display, 0);

   glDisable(GL_CULL_FACE);
   glDisable(GL_DEPTH_TEST);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   // Load shaders:
   printGLString("Version", GL_VERSION);
   printGLString("Vendor", GL_VENDOR);
   printGLString("Renderer", GL_RENDERER);
   printGLString("Extensions", GL_EXTENSIONS);

   LOGI("setupGraphics(%d, %d)", viewportWidth, viewportHeight);
      
   // Setup Window
   if(!simulation->setup(w, h, viewportWidth, viewportHeight, *assetLoader) )
   {
      LOGE("Failed to setup simulation");
      return false;
   }

   return true;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame() 
{
    if(display == NULL)
        return;

   const std::clock_t start = std::clock();   
   simulation->tick();
   
   // Render to screen
   eglSwapBuffers(display, surface);    
   frameTime = (std::clock() - start) / (float)CLOCKS_PER_SEC;
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display()
{
   if(display != EGL_NO_DISPLAY)
   {
      eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      if(context != EGL_NO_CONTEXT)
      {
         eglDestroyContext(display, context);
      }

      if(surface != EGL_NO_SURFACE) 
      {
         eglDestroySurface(display, surface);
      }

      eglTerminate(display);
   }

   animating = 0;
   display = EGL_NO_DISPLAY;
   context = EGL_NO_CONTEXT;
   surface = EGL_NO_SURFACE;

   delete simulation;
   simulation = 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   // Prepare infrastructure
   simulation = new Simulation();
   assetLoader = new WindowsAssetLoader();

   if(!engine_init_display(hInstance))
      return -1;

   MSG message;
   message.message = (~WM_QUIT);

   // Loop until a WM_QUIT message is received
   while(message.message != WM_QUIT)
   {
      if(PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
      {
         // If a message was waiting in the message queue, process it
         TranslateMessage(&message);
         DispatchMessage(&message);
      }
      else
      {
         engine_draw_frame();
      }
   }
   
   return 0;
}
