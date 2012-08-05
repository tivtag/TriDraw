# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SOURCE_PATH = $(LOCAL_PATH)/../../../../src/
FILE_LIST := $(wildcard $(SOURCE_PATH)*.cpp)

LOCAL_CFLAGS 	+= -std=gnu++0x
LOCAL_CFLAGS 	+= -Werror -DBOOST_EXCEPTION_DISABLE -D_STLP_NO_EXCEPTIONS -DOS_ANDROID -D_STLP_USE_SIMPLE_NODE_ALLOC -D_LITTLE_ENDIAN -O3 

LOCAL_MODULE    := tri-draw
LOCAL_SRC_FILES := $(SOURCE_PATH)main-android.cpp $(SOURCE_PATH)Image.cpp $(SOURCE_PATH)Texture.cpp $(SOURCE_PATH)TexturedQuadEffect.cpp $(SOURCE_PATH)stb_image.cpp $(SOURCE_PATH)Effect.cpp $(SOURCE_PATH)AndroidAssetLoader.cpp $(SOURCE_PATH)Simulation.cpp
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_C_INCLUDES := $(LOCAL_PATH)/dependencies

# LOCAL_CFLAGS += "-IC:/Programming/android-ndk-r8/sources/boost"


include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
