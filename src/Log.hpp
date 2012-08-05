#pragma once

#if defined(WIN32)
#  include <cstdio>
#  ifdef DEBUG
#     define LOGI(...) printf(__VA_ARGS__); printf("\r\n")
#  else
#     define LOGI(...) 
#  endif
#  define LOGW(...) printf(__VA_ARGS__); printf("\r\n")
#  define LOGE(...) printf(__VA_ARGS__); printf("\r\n")
#else
#  include <android/log.h>
#  ifdef DEBUG
#     define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "tri-draw", __VA_ARGS__))
#  else
#     define LOGI(...) 
#endif
#  define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "tri-draw", __VA_ARGS__))
#  define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "tri-draw", __VA_ARGS__))
#endif
