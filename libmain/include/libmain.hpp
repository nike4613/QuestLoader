#pragma once

#include <jni.h>
#include <string_view>
#include "libmain/utils.hpp"
#include "libmain/_config.hpp"

#define CHECK_MODLOADER_FUNCTION(funcname) \
    static_assert(::std::is_same_v<std::remove_reference_t<decltype(&modloader_##funcname)>, ::jni::modloader::funcname##_t*>, \
    "modloader_main either has the wrong signature, or does not exist!")

#define CHECK_MODLOADER_MAIN CHECK_MODLOADER_FUNCTION(main)
#define CHECK_MODLOADER_ACCEPT_UNITY_HANDLE CHECK_MODLOADER_FUNCTION(accept_unity_handle)

namespace jni {

    namespace modloader {
        // first, main is called. the interface it returns is given to LibUnity, but only after calling accept_unity_handle.
        using main_t = JNINativeInterface(JavaVM* vm, JNIEnv* env, std::string_view loadSrc) noexcept;
        // this is called *before* calling LibUnity's JNI_OnLoad.
        using accept_unity_handle_t = void(void* unityModuleHandle) noexcept;
    }

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
            static constexpr void* JNINativeInterface::* extra_member = &JNINativeInterface::reserved2;
        };
        template<>
        struct interface_store_members<JNIInvokeInterface> {
            static constexpr void* JNIInvokeInterface::* original_member = &JNIInvokeInterface::reserved0;
            static constexpr void* JNIInvokeInterface::* user_member = &JNIInvokeInterface::reserved1;
            static constexpr void* JNIInvokeInterface::* extra_member = &JNIInvokeInterface::reserved2;
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
        template<typename U, typename T>
        U*& interface_extra(T* i) noexcept
        { return reinterpret_cast<U*&>(i->*(interface_store_members<T>::extra_member)); }

        template<typename Iface, typename FType, typename ...Args>
        auto invoke_original(Iface** self, FType* Iface::* member, Args&& ...args) {
            auto original = interface_original(*self);
            return ((*original)->*member)(original, std::forward<Args>(args)...);
        }
        template<typename IfaceWrap, typename FType, typename ...Args>
        auto invoke_original(IfaceWrap* self, FType* std::remove_pointer_t<std::remove_reference_t<decltype(self->functions)>>::* member, Args&& ...args) {
            using IfacePtr = std::remove_reference_t<decltype(self->functions)>;
            auto original = interface_original(*reinterpret_cast<IfacePtr*>(self));
            return ((*original)->*member)(reinterpret_cast<IfaceWrap*>(original), std::forward<Args>(args)...);
        }
    }
}