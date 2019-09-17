#include <libmain.hpp>
#include "log.hpp"
#include <cstddef>
#include <cstdint>

// holy fuck why won't this export
extern "C" JNINativeInterface modloader_main(JavaVM* vm, JNIEnv* env, std::string_view loadSrc) noexcept {
    logf(ANDROID_LOG_VERBOSE, "modloader_main called with vm: 0x%p, env: 0x%p, loadSrc: %s", vm, env, loadSrc.data());

    return jni::interface::make_passthrough_interface<JNINativeInterface>(&env->functions);
}

CHECK_MODLOADER_MAIN;