#pragma once

#include <android/log.h>

#define log(priority, message) __android_log_print(priority, "libmain - patched", message)
#define logf(priority, ...) __android_log_print(priority, "libmain - patched", __VA_ARGS__)