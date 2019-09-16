#include <jni.h>
#include <array>

#include "log.hpp"
#include "libmain.hpp"

#include <cstddef>

constexpr std::array<JNINativeMethod, 2> NativeLoader_bindings = {{
    { "load",   "(Ljava/lang/String;)Z", (void*)&jni::load   },
    { "unload", "()Z",                   (void*)&jni::unload }
}};

extern "C" jint LIBMAIN_EXPORT JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env = nullptr;

    log(ANDROID_LOG_INFO, "JNI_OnLoad called, linking JNI methods");

    vm->AttachCurrentThread(&env, nullptr);
    auto klass = env->FindClass("com/unity3d/player/NativeLoader");

    auto ret = env->RegisterNatives(klass, NativeLoader_bindings.data(), NativeLoader_bindings.size());

    if (ret < 0) {
        logf(ANDROID_LOG_WARN, "RegisterNatives failed with %d", ret);

        env->FatalError("com/unity3d/player/NativeLoader"); // this is such a useless fucking error message because the original libmain does this

        return -1;
    }

    log(ANDROID_LOG_INFO, "JNI_OnLoad done!");

    return JNI_VERSION_1_6;
}

extern "C" void LIBMAIN_EXPORT JNI_OnUnload(JavaVM* vm, void*) {
    log(ANDROID_LOG_INFO, "JNI_OnUnload called");
}