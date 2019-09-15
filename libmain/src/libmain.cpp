#include "libmain.hpp"
#include "log.hpp"

/*
    This file defines equivalent functionality to stock libmain, with some extra logging.

    All custom functionality should be done elsewhere.
*/

#include <dlfcn.h>

#include <memory>
#include <string_view>
#include <array>

using namespace std::literals;

static void* libUnityHandle = nullptr;

jboolean jni::NativeLoader::load(JNIEnv* env, jobject klass, jstring str) noexcept {
    auto const len = env->GetStringUTFLength(str);

    constexpr auto sobasename = "libunity.so"sv;

    char soname[len + 1 + sobasename.length() + 1]; // use stack local to prevent allocation
    soname[len] = 0;

    {
        auto chars = env->GetStringUTFChars(str, nullptr);
        std::copy(chars, chars + len, soname); // instead of using memcpy
        env->ReleaseStringUTFChars(str, chars);
    }

    if (libUnityHandle != nullptr)
        return true;

    JavaVM* vm = nullptr;

    if (env->GetJavaVM(&vm) < 0) {
        env->FatalError("Unable to retrieve Java VM"); // libmain wording
        return false; // doesn't actually reach here
    }

    soname[len] = '/';
    // because std::copy returns the element after the last copied, which is what we want to be null
    *std::copy(std::begin(sobasename), std::end(sobasename), soname + len + 1) = 0;

    libUnityHandle = dlopen(soname, RTLD_LAZY);
    if (libUnityHandle == nullptr) {
        libUnityHandle = dlopen(sobasename.data(), RTLD_LAZY);
        if (libUnityHandle == nullptr) {
            auto err = dlerror();
            logf(ANDROID_LOG_WARN, "Could not load libunity.so from %s: %s", soname, err);
            std::array<char, 0x400> message = {0}; // this is what the original libmain allocates, so lets hope its enough
                                                   // and doesn't use all of the rest of our stack space
            snprintf(message.data(), message.size(), "Unable to load library: %s [%s]", soname, err);
            env->FatalError(message.data());
            return false; // doesn't actually reach here
        }
    }

    using JNI_OnLoad_t = jint(JavaVM*, void*);

    auto onload = reinterpret_cast<JNI_OnLoad_t*>(dlsym(libUnityHandle, "JNI_OnLoad"));
    if (onload != nullptr) {
        if (onload(vm, nullptr) > JNI_VERSION_1_6) {
            log(ANDROID_LOG_WARN, "libunity JNI_OnLoad requested unsupported VM version");
            env->FatalError("Unsupported VM version"); // libmain wording
            return false; // doesn't actually reach here
        }
    } else {
        log(ANDROID_LOG_INFO, "libunity does not have a JNI_OnLoad");
    }

    logf(ANDROID_LOG_INFO, "Successfully loaded and initialized %s", soname);

    return libUnityHandle != nullptr;
}

jboolean jni::NativeLoader::unload(JNIEnv* env, jobject klass) noexcept {
    JavaVM* vm = nullptr;

    if (env->GetJavaVM(&vm) < 0) {
        env->FatalError("Unable to retrieve Java VM"); // libmain wording
        return false; // doesn't actually reach here
    }

    using JNI_OnUnload_t = jint(JavaVM*, void*);

    auto onunload = reinterpret_cast<JNI_OnUnload_t*>(dlsym(libUnityHandle, "JNI_OnUnload"));
    if (onunload != nullptr) {
        onunload(vm, nullptr);
    } else {
        log(ANDROID_LOG_INFO, "libunity does not have a JNI_OnUnload");
    }

    int code = dlclose(libUnityHandle);

    if (code != 0) {
        logf(ANDROID_LOG_WARN, "Error occurred closing libunity: %s", dlerror());
    } else {
        log(ANDROID_LOG_INFO, "Successfully closed libunity");
    }

    return true;
}