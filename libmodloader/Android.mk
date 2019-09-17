# Copyright (C) 2009 The Android Open Source Project
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
#
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE            := main
LOCAL_SRC_FILES         := ../libmain/libs/$(TARGET_ARCH_ABI)/libmain.so
LOCAL_EXPORT_C_INCLUDES := ../libmain/include
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

LOCAL_LDLIBS    := -llog
LOCAL_MODULE    := modloader
LOCAL_CPPFLAGS  := -std=c++2a -fno-rtti -Os

LOCAL_SHARED_LIBRARIES := main
LOCAL_C_INCLUDES := ./include ./src 
LOCAL_SRC_FILES  := $(call rwildcard,src/,*.cpp)

include $(BUILD_SHARED_LIBRARY)