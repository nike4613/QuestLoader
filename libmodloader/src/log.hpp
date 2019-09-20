#pragma once

#include <android/log.h>

#define log(priority, message) __android_log_print(priority, "libmodloader", message)
#define logf(priority, ...) __android_log_print(priority, "libmodloader", __VA_ARGS__)