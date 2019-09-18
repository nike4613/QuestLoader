#include "libmain.hpp"
#include "log.hpp"

/*
    This file defines equivalent functionality to stock libmain, with some extra logging.

    All custom functionality should be done elsewhere.
*/

#include <dlfcn.h>

#include <memory>
#include <array>
#include <map>

using namespace std::literals;
using namespace jni;

static std::map<JNIEnv*, JNIEnv*> envPtrs = {};

static void* libModLoader = nullptr;
static void* libUnityHandle = nullptr;

// this is needed because libUnity stores the pointer, so this can never be invalidated
static JNINativeInterface libUnityNInterface = {};
static JNIEnv libUnityEnv = {&libUnityNInterface};
static JNIInvokeInterface libUnityIInterface = {};
static JavaVM libUnityVm = {&libUnityIInterface};
static bool usingCustomVm = false;

jboolean jni::load(JNIEnv* env, jobject klass, jstring str) noexcept {
    auto const len = env->GetStringUTFLength(str);

    constexpr auto unityso = "libunity.so"sv;
    constexpr auto modloaderso = "libmodloader.so"sv;
    constexpr auto sonameLen = std::max(unityso.length(), modloaderso.length());

    char soname[len + 1 + sonameLen + 1]; // use stack local to prevent allocation
    soname[len] = 0;

    {
        auto chars = env->GetStringUTFChars(str, nullptr);
        std::copy(chars, chars + len, soname); // instead of using memcpy
        env->ReleaseStringUTFChars(str, chars);
    }

    if (libUnityHandle != nullptr)
        return true;

    logf(ANDROID_LOG_VERBOSE, "Searching in %s", soname);

    JavaVM* vm = nullptr;

    if (env->GetJavaVM(&vm) < 0) {
        env->FatalError("Unable to retrieve Java VM"); // libmain wording
        return false; // doesn't actually reach here
    }

    auto unityVm = vm;

    log(ANDROID_LOG_VERBOSE, "Got JVM");

    { // try load libmodloader
        soname[len] = '/';
        // because std::copy returns the element after the last copied, which is what we want to be null
        auto endptr = std::copy(std::begin(modloaderso), std::end(modloaderso), soname + len + 1);
        *endptr = 0;
        
        logf(ANDROID_LOG_VERBOSE, "Looking for libmodloader at %s", soname);

        libModLoader = dlopen(soname, RTLD_LAZY);
        if (libModLoader == nullptr) {
            logf(ANDROID_LOG_VERBOSE, "Looking for libmodloader as %s", modloaderso.data());
            libModLoader = dlopen(modloaderso.data(), RTLD_LAZY);
            if (libUnityHandle == nullptr) {
                auto err = dlerror();
                logf(ANDROID_LOG_WARN, "Could not load libmodloader.so from %s: %s", soname, err);
                goto loadLibUnity;
            }
        }

        log(ANDROID_LOG_VERBOSE, "Loaded libmodloader");

        auto main = reinterpret_cast<modloader_main_t*>(dlsym(libModLoader, "modloader_main"));
        if (main == nullptr) {
            logf(ANDROID_LOG_WARN, "libmodloader does not have modloader_main: %s", dlerror());
            goto loadLibUnity;
        }

        log(ANDROID_LOG_VERBOSE, "Using libmodloader's modloader_main");
        libUnityNInterface = main(vm, env, soname);

        usingCustomVm = true;
        unityVm = &libUnityVm;
        libUnityIInterface = interface::make_passthrough_interface<JNIInvokeInterface>(&vm->functions);
        libUnityIInterface.AttachCurrentThread = [](JavaVM* ptr, JNIEnv** envp, void* aarg) {
            auto orig = interface::interface_original(ptr->functions);
            
            JNIEnv* env;
            auto ret = (*orig)->AttachCurrentThread(reinterpret_cast<JavaVM*>(orig), &env, aarg);
            if (ret) return ret;

            auto eptr = envPtrs.find(env);
            if (eptr == envPtrs.end()) {
                // the JNIEnv ends up being stored in the extra reserved member.
                auto interf = new JNINativeInterface(libUnityNInterface);
                interface::interface_extra<JNINativeInterface>(interf) = interf;
                auto envptr = reinterpret_cast<JNIEnv*>(&interface::interface_extra<JNINativeInterface>(interf));
                interface::interface_original(interf) = const_cast<JNINativeInterface**>(&env->functions);

                eptr = envPtrs.insert({env, envptr}).first;
            }

            *envp = eptr->second;
            return ret;
        };
        libUnityIInterface.AttachCurrentThreadAsDaemon = [](JavaVM* ptr, JNIEnv** envp, void* aarg) {
            auto orig = interface::interface_original(ptr->functions);
            
            JNIEnv* env;
            auto ret = (*orig)->AttachCurrentThreadAsDaemon(reinterpret_cast<JavaVM*>(orig), &env, aarg);
            if (ret) return ret;

            auto eptr = envPtrs.find(env);
            if (eptr == envPtrs.end()) {
                // the JNIEnv ends up being stored in the extra reserved member.
                auto interf = new JNINativeInterface(libUnityNInterface);
                interface::interface_extra<JNINativeInterface>(interf) = interf;
                auto envptr = reinterpret_cast<JNIEnv*>(&interface::interface_extra<JNINativeInterface>(interf));
                interface::interface_original(interf) = const_cast<JNINativeInterface**>(&env->functions);

                eptr = envPtrs.insert({env, envptr}).first;
            }

            *envp = eptr->second;
            return ret;
        };
        libUnityIInterface.GetEnv = [](JavaVM* ptr, void** envp, jint ver) {
            auto orig = interface::interface_original(ptr->functions);
            
            JNIEnv* env;
            auto ret = (*orig)->GetEnv(reinterpret_cast<JavaVM*>(orig), reinterpret_cast<void**>(&env), ver);
            if (ret) return ret;

            auto eptr = envPtrs.find(env);
            if (eptr == envPtrs.end()) {
                // the JNIEnv ends up being stored in the extra reserved member.
                auto interf = new JNINativeInterface(libUnityNInterface);
                interface::interface_extra<JNINativeInterface>(interf) = interf;
                auto envptr = reinterpret_cast<JNIEnv*>(&interface::interface_extra<JNINativeInterface>(interf));
                interface::interface_original(interf) = const_cast<JNINativeInterface**>(&env->functions);

                eptr = envPtrs.insert({env, envptr}).first;
            }

            *envp = eptr->second;
            return ret;
        };
    }

    loadLibUnity:
    { // try load libunity
        soname[len] = '/';
        // because std::copy returns the element after the last copied, which is what we want to be null
        auto endptr = std::copy(std::begin(unityso), std::end(unityso), soname + len + 1);
        *endptr = 0;

        libUnityHandle = dlopen(soname, RTLD_LAZY);
        if (libUnityHandle == nullptr) {
            libUnityHandle = dlopen(unityso.data(), RTLD_LAZY);
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
            if (onload(unityVm, nullptr) > JNI_VERSION_1_6) {
                log(ANDROID_LOG_WARN, "libunity JNI_OnLoad requested unsupported VM version");
                env->FatalError("Unsupported VM version"); // libmain wording
                return false; // doesn't actually reach here
            }
        } else {
            log(ANDROID_LOG_INFO, "libunity does not have a JNI_OnLoad");
        }
    }

    { // TODO: add call into libmodloader with libunity.so handle

    }

    logf(ANDROID_LOG_INFO, "Successfully loaded and initialized %s", soname);

    return libUnityHandle != nullptr;
}

jboolean jni::unload(JNIEnv* env, jobject klass) noexcept {
    JavaVM* vm = nullptr;

    if (env->GetJavaVM(&vm) < 0) {
        env->FatalError("Unable to retrieve Java VM"); // libmain wording
        return false; // doesn't actually reach here
    }

    using JNI_OnUnload_t = jint(JavaVM*, void*);

    auto onunload = reinterpret_cast<JNI_OnUnload_t*>(dlsym(libUnityHandle, "JNI_OnUnload"));
    if (onunload != nullptr) {
        onunload(usingCustomVm ? &libUnityVm : vm, nullptr);
    } else {
        log(ANDROID_LOG_WARN, "libunity does not have a JNI_OnUnload");
    }

    int code = dlclose(libUnityHandle);

    if (code != 0) {
        logf(ANDROID_LOG_WARN, "Error occurred closing libunity: %s", dlerror());
    } else {
        log(ANDROID_LOG_VERBOSE, "Successfully closed libunity");
    }

    if (libModLoader != nullptr) {
        code = dlclose(libModLoader);
        if (code != 0) {
            logf(ANDROID_LOG_WARN, "Error occurred closing libModLoader: %s", dlerror());
        } else {
            log(ANDROID_LOG_VERBOSE, "Successfully closed libModLoader");
        }
    }

    return true;
}