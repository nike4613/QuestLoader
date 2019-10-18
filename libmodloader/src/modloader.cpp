#include <libmain.hpp>
#include "log.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

#include "jit/jit.hpp"

#include <sys/mman.h>

using namespace modloader;

namespace {
    // there is *not* a good way to get the name lmao
    std::string get_jni_class_name(JNIEnv* env, jclass klass) {
        auto kl = env->GetObjectClass(klass);
        auto mid = env->GetMethodID(kl, "getName", "()Ljava/lang/String;");
        auto jstr = static_cast<jstring>(env->CallObjectMethod(klass, mid));

        auto cstr = env->GetStringUTFChars(jstr, nullptr);
        std::string name {cstr};
        env->ReleaseStringUTFChars(jstr, cstr);

        return name;
    }
}

extern "C" void modloader_preload() noexcept {
    log(ANDROID_LOG_VERBOSE, "modloader_preload called (should be really early)");
}

extern "C" JNINativeInterface modloader_main(JavaVM* vm, JNIEnv* env, std::string_view loadSrc) noexcept {
    logf(ANDROID_LOG_VERBOSE, "modloader_main called with vm: 0x%p, env: 0x%p, loadSrc: %s", vm, env, loadSrc.data());

    auto iface = jni::interface::make_passthrough_interface<JNINativeInterface>(&env->functions);

    iface.RegisterNatives = [](JNIEnv* env, jclass klass, JNINativeMethod const* methods_ptr, jint count) {
        using namespace jni::interface;
        std::span methods {const_cast<JNINativeMethod*>(methods_ptr), count};
        int success = mem::protect(methods, mem::protection::read_write_execute); // ensure the protection is right
        // the reason such a broad protection level is set is so that i don't accidentally mark some bit of code not executable

        logf(ANDROID_LOG_DEBUG, "mem::protect returned %d", success);

        // call it with interface_original so any modifications previously made to *this* JNIEnv don't affect it
        auto clsname = get_jni_class_name(interface_original(*env), klass);

        for (auto& method : methods) {
            logf(ANDROID_LOG_VERBOSE, "Unity registering native on %s: %s %s @ 0x%p", 
                    clsname.data(), method.name, method.signature, method.fnPtr);
            method.fnPtr = jit::make_native_wrapper(method.fnPtr, method.name);
        }

        return invoke_original(env, &JNINativeInterface::RegisterNatives, klass, methods_ptr, count);
    };

    return iface;
}

extern "C" void modloader_accept_unity_handle(void* uhandle) noexcept {
    logf(ANDROID_LOG_VERBOSE, "modloader_accept_unity_handle called with uhandle: 0x%p", uhandle);
}

CHECK_MODLOADER_PRELOAD;
CHECK_MODLOADER_MAIN;
CHECK_MODLOADER_ACCEPT_UNITY_HANDLE;