
#include <vector>

#include <jni.h>
#include <android/sensor.h>
#include <android/window.h>
#include <android_native_app_glue.h>

#include "Log.hpp"
#include "OpenGL.hpp"
#include "Random.hpp"

#include "Simulation.hpp"
#include "AndroidAssetLoader.hpp"

Simulation simulation;
AssetLoader* assetLoader = 0;

GLuint viewportWidth = -1;
GLuint viewportHeight = -1;
float frameTime = 0.0f;

/**
 * Shared state for our app.
 */
class engine 
{
public:
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    const ASensor* gyroSensor;
    ASensorEventQueue* sensorEventQueue;
    AAssetManager* assetManager;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
};

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) 
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
    
   EGLint w, h, dummy, format;
   EGLint numConfigs;
   EGLConfig config;
   EGLSurface surface;
   EGLContext context;

   EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
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

   ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

   surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
   context = eglCreateContext(display, config, NULL, attrib_list);

   if(eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
      LOGW("Unable to eglMakeCurrent");
      return -1;
   }

   eglQuerySurface(display, surface, EGL_WIDTH, &w);
   eglQuerySurface(display, surface, EGL_HEIGHT, &h);

   engine->display = display;
   engine->context = context;
   engine->surface = surface;
   viewportWidth = w / Constants::DownScaleFactor;
   viewportHeight = h /  Constants::DownScaleFactor;

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

   LOGW("setupGraphics view=(%d, %d) win=(%d, %d)", viewportWidth, viewportHeight, w, h);
              
   if(!simulation.setup(w, h, viewportWidth, viewportHeight, *assetLoader) )
   {
      LOGE("Failed to setup simulation");
      return -1;
   }

   return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) 
{
    if(engine->display == NULL)
        return;

   //const std::clock_t start = std::clock();   
   simulation.tick();
   
	// Render to screen
   eglSwapBuffers(engine->display, engine->surface);    
   //frameTime = (std::clock() - start) / (float)CLOCKS_PER_SEC;
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine)
{
   LOGW("Terminating..");

   if(engine->display != EGL_NO_DISPLAY)
   {
      eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      if(engine->context != EGL_NO_CONTEXT)
      {
         eglDestroyContext(engine->display, engine->context);
      }

      if(engine->surface != EGL_NO_SURFACE) 
      {
         eglDestroySurface(engine->display, engine->surface);
      }
      eglTerminate(engine->display);
   }

   engine->animating = 0;
   engine->display = EGL_NO_DISPLAY;
   engine->context = EGL_NO_CONTEXT;
   engine->surface = EGL_NO_SURFACE;
   
   ANativeActivity_finish(engine->app->activity);
   exit(0);
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
   struct engine* engine = (struct engine*)app->userData;

   if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
   {
      if(AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_DOWN)
      {
         simulation.toggleOutputTexture();
      }   

      engine->animating = 1;
      // engine->state.x = AMotionEvent_getX(event, 0);
      // engine->state.y = AMotionEvent_getY(event, 0);
      return 1;
   }

   return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
   struct engine* engine = (struct engine*)app->userData;

   switch(cmd) 
	{
        case APP_CMD_SAVE_STATE:
            break;

        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if(engine->app->window != NULL)
			   {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;

        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;

        case APP_CMD_GAINED_FOCUS:
            //// When our app gains focus, we start monitoring the accelerometer.
            //if (engine->accelerometerSensor != NULL) {
            //    ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
            //    // We'd like to get 60 events per second (in us).
            //    ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L/60)*1000);
            //    
            //    ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->gyroSensor);
            //    // We'd like to get 60 events per second (in us).
            //    ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->gyroSensor, (1000L/60)*1000);                
            //}
            engine->animating = 1;
            break;

        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            //if (engine->accelerometerSensor != NULL) {
            //    ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
            //}
            //        if (engine->gyroSensor != NULL) {
            //    ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->gyroSensor);
            //}

            // Also stop animating.
            // engine->animating = 0;
            // engine_draw_frame(engine);
			engine_term_display(engine);
			ANativeActivity_finish(app->activity);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) 
{
   struct engine engine;

   // Make sure glue isn't stripped.
   app_dummy();

   memset(&engine, 0, sizeof(engine));
   state->userData = &engine;
   state->onAppCmd = engine_handle_cmd;
   state->onInputEvent = engine_handle_input;
   engine.app = state;
   
   ANativeActivity_setWindowFlags(state->activity, AWINDOW_FLAG_FULLSCREEN|AWINDOW_FLAG_KEEP_SCREEN_ON , 1);

   // Prepare infrastructure
   setup_rand();
   assetLoader = new AndroidAssetLoader(state->activity->assetManager);

   // Prepare sensors
   engine.assetManager = state->activity->assetManager;
   engine.sensorManager = ASensorManager_getInstance();
   //engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
   //engine.gyroSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_GYROSCOPE);
   //engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);
      
   // loop waiting for stuff to do.
   while(1) 
   {
      // Read all pending events.
      int ident;
      int events;
      struct android_poll_source* source;

      // If not animating, we will block forever waiting for events.
      // If animating, we loop until all events are read, then continue
      // to draw the next frame of animation.
      while((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
      {
         // Process this event.
         if(source != NULL)
         {
            source->process(state, source);
         }

         // If a sensor has data, process it now.
         if(ident == LOOPER_ID_USER)
         {
         }

         // Check if we are exiting.
         if(state->destroyRequested != 0)
         {
            engine_term_display(&engine);
            return;
         }
      }

      if(engine.animating)
      {
         engine_draw_frame(&engine);
      }
   }
}
