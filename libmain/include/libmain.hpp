#ifndef LIBMAIN_HPP
#define LIBMAIN_HPP

#include <jni.h>
#include <string_view>
#include "libmain/utils.hpp"

#define LIBMAIN_EXPORT __attribute__((visibility("default")))
#define CHECK_MODLOADER_MAIN \
    static_assert(::std::is_same_v<std::remove_reference_t<decltype(&modloader_main)>, ::jni::modloader_main_t*>, \
    "modloader_main either has the wrong signature, or does not exist!")

namespace jni {

    using modloader_main_t = JNINativeInterface(JavaVM* vm, JNIEnv* env, std::string_view loadSrc) noexcept;

    jboolean load(JNIEnv* env, jobject klass, jstring str) noexcept;
    jboolean unload(JNIEnv* env, jobject klass) noexcept;

    template<typename R, typename ...A>
    using function = R(A...);

    // both of these use the first reserved slot to hold the original; you should use the second for other data
    namespace interface {
        // holy fuck all of this is terrible
        // please someone tell me a better way to do this

        template<typename Interface>
        Interface LIBMAIN_EXPORT make_passthrough_interface(Interface const* const* i) noexcept;

        template<typename> struct interface_store_members;
        template<>
        struct interface_store_members<JNINativeInterface> {
            static constexpr void* JNINativeInterface::* original_member = &JNINativeInterface::reserved0;
            static constexpr void* JNINativeInterface::* user_member = &JNINativeInterface::reserved1;
        };
        template<>
        struct interface_store_members<JNIInvokeInterface> {
            static constexpr void* JNIInvokeInterface::* original_member = &JNIInvokeInterface::reserved0;
            static constexpr void* JNIInvokeInterface::* user_member = &JNIInvokeInterface::reserved1;
        };

        template<typename T>
        T**& interface_original(T* i) noexcept
        { return reinterpret_cast<T**&>((i)->*(interface_store_members<T>::original_member)); }
        template<typename T>
        T**const& interface_original(T const* i) noexcept 
        { return reinterpret_cast<T**const&>((i)->*(interface_store_members<T>::original_member)); }
        template<typename U, typename T>
        U*& interface_user(T* i) noexcept
        { return reinterpret_cast<U*&>(i->*(interface_store_members<T>::user_member)); }
    }
}

#endif