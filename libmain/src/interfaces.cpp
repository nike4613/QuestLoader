#include "libmain.hpp"
#include "log.hpp"

using namespace jni::interface;

// TODO: eliminate namespace name
namespace detail {
    template<typename> struct func_ptr_helper;
    template<typename R, typename ...Args>
    struct func_ptr_helper<R(*)(Args...)> {
        using ret = R;
        using args = util::type_list<Args...>;
        static constexpr bool is_varargs = false;
    };
    template<typename R, typename ...Args>
    struct func_ptr_helper<R(*)(Args..., ...)> {
        using ret = R;
        using args = util::type_list<Args...>;
        static constexpr bool is_varargs = true;
    };
    
    template<typename... Args>
    struct make_passthrough_helper {
        template<typename O, typename T, typename F, F O::* member, bool debug_print = false, typename NameProvider = void>
        static auto varargs_form(T* o, Args... args, ...) { // this case cannot be a lambda because Clang cannot compile C varargs lambdas
            auto op = *reinterpret_cast<O**>(o); // Debug logging is omitted because this vararg is already probably broken
            return ((*interface_original(op))->*member)( // TODO: figure out how to actually wrap varargs properly (inline assembly?)      
                reinterpret_cast<T*>(interface_original(op)), args...);
        }
        template<typename O, typename T, typename F, F O::* member, bool is_varargs, bool debug_print = false, typename NameProvider = void>
        static auto get_functor() noexcept {
            if constexpr (is_varargs) {
                return &varargs_form<O, T, F, member, debug_print, NameProvider>;
            } else {
                return [](T* o, Args... args) {
                    if constexpr (debug_print) {
                        logf(ANDROID_LOG_DEBUG, "Invoking wrapped JNI function %s for %p", NameProvider{}.name, o);
                    }
                    auto op = *reinterpret_cast<O**>(o);
                    return ((*interface_original(op))->*member)(
                        reinterpret_cast<T*>(interface_original(op)), args...);
                };
            }
        }
    };
}

template<typename Obj, typename OwnType, typename FType, FType Obj::* member, bool debug_print = false, typename NameProvider = void>
auto make_passthrough() noexcept -> 
    util::alias_t<
        std::enable_if_t<
            std::is_same_v<
                typename detail::func_ptr_helper<FType>::args::first,
                OwnType*
            >,
            FType
        >,
        typename detail::func_ptr_helper<FType>::ret,
        typename detail::func_ptr_helper<FType>::args
    >
{
    return detail::func_ptr_helper<FType>::args::rest
            ::template apply<detail::make_passthrough_helper>
            ::template get_functor<Obj, OwnType, FType, member, detail::func_ptr_helper<FType>::is_varargs, debug_print, NameProvider>();
}


namespace {
    template<typename R, typename ...Args>
    using variadic_ptr = R(*)(Args..., ...);
    template<typename R, typename ...Args>
    using func_ptr = R(*)(Args...);
    template<typename Own, typename R, typename ...Args>
    using member_ptr = R(* Own::*)(Args...);

    template<typename Own>
    struct make_membr {
        template<typename R, typename ...Args>
        using ptr = member_ptr<Own, R, Args...>;
    };

    template<template<typename,typename...>typename F, typename R>
    struct make_fptr {
        template<typename ...Args>
        using ptr = F<R, Args...>;
    };

    template<typename O, typename T>
    using as_member = T O::*;
}

#define INTERFACE_DEBUG

#ifdef INTERFACE_DEBUG
#  define PASSTHROUGH_DEBUG_ARGS(member) , true, NameProvider_##member
#else
#  define PASSTHROUGH_DEBUG_ARGS(member)
#endif

#define PASSTHROUGH_(Interface, OwnType, var, member) var.member = make_passthrough<Interface, OwnType, std::remove_reference_t<decltype(var.member)>, \
                                                                                    &Interface::member PASSTHROUGH_DEBUG_ARGS(member)>()

#ifdef INTERFACE_DEBUG
#  define PASSTHROUGH(Interface, OwnType, var, member) struct NameProvider_##member { char const* name = #member; }; \
                                                       PASSTHROUGH_(Interface, OwnType, var, member)
#else
#  define PASSTHROUGH(Interface, OwnType, var, member) PASSTHROUGH_(Interface, OwnType, var, member)
#endif

#define E(...) __VA_ARGS__

template<>
JNIInvokeInterface LIBMAIN_EXPORT jni::interface::make_passthrough_interface(JNIInvokeInterface const*const* original) noexcept {
    JNIInvokeInterface i;
    interface_original(&i) = const_cast<JNIInvokeInterface**>(original);

    #define M(member) PASSTHROUGH(JNIInvokeInterface, JavaVM, i, member)

    M(GetEnv); M(DestroyJavaVM); 
    M(AttachCurrentThread); M(AttachCurrentThreadAsDaemon);
    M(DetachCurrentThread);

    #undef M

    return i;
}


template<>
JNINativeInterface LIBMAIN_EXPORT jni::interface::make_passthrough_interface(JNINativeInterface const*const* original) noexcept {
    JNINativeInterface i;
    interface_original(&i) = const_cast<JNINativeInterface**>(original);

    #define M(member) PASSTHROUGH(JNINativeInterface, JNIEnv, i, member)
    #define N(name) M(name); M(name##V); M(name##A);

    #define ObjTypesNO(O, pre, post) \
        O(pre ## Boolean ## post); \
        O(pre ## Byte ## post); \
        O(pre ## Char ## post); \
        O(pre ## Short ## post); \
        O(pre ## Int ## post); \
        O(pre ## Long ## post); \
        O(pre ## Float ## post); \
        O(pre ## Double ## post);
    #define ObjTypes(O, pre, post) \
        O(pre ## Object ## post); \
        ObjTypesNO(O, pre, post);
    #define ObjTypesV(O, pre, post) \
        ObjTypes(O, pre, post); \
        O(pre ## Void ## post);

    M(GetVersion); M(DefineClass); M(FindClass);
    M(FromReflectedMethod); M(FromReflectedField);
    M(ToReflectedMethod); M(ToReflectedField);
    M(GetSuperclass); M(IsAssignableFrom);
    M(Throw); M(ThrowNew); M(GetObjectRefType);
    M(ExceptionOccurred); M(ExceptionDescribe);
    M(ExceptionClear); M(FatalError);
    M(PushLocalFrame); M(PopLocalFrame);
    M(NewGlobalRef); M(DeleteGlobalRef); M(DeleteLocalRef);
    M(IsSameObject); M(NewLocalRef); M(EnsureLocalCapacity);
    M(AllocObject); N(NewObject);
    M(GetObjectClass); M(IsInstanceOf); M(GetMethodID);
    M(GetFieldID); M(GetStaticMethodID); M(GetStaticFieldID);
    M(NewString); M(GetStringLength);
    M(GetStringChars); M(ReleaseStringChars);
    M(NewStringUTF); M(GetStringUTFLength); M(GetStringUTFChars);
    M(ReleaseStringUTFChars); M(GetArrayLength);
    M(GetObjectArrayElement); M(SetObjectArrayElement);
    M(RegisterNatives); M(UnregisterNatives);
    M(MonitorEnter); M(MonitorExit); M(GetJavaVM);
    M(GetStringRegion); M(GetStringUTFRegion);
    M(GetPrimitiveArrayCritical); M(ReleasePrimitiveArrayCritical);
    M(GetStringCritical); M(ReleaseStringCritical);
    M(NewWeakGlobalRef); M(DeleteWeakGlobalRef);
    M(ExceptionCheck); M(NewDirectByteBuffer);
    M(GetDirectBufferAddress); M(GetDirectBufferCapacity);

    ObjTypesV(N, Call, Method);
    ObjTypesV(N, CallNonvirtual, Method);
    ObjTypesV(N, CallStatic, Method);
    ObjTypesNO(M, Get, ArrayElements);
    ObjTypesNO(M, Release, ArrayElements);
    ObjTypesNO(M, Get, ArrayRegion);
    ObjTypesNO(M, Set, ArrayRegion);
    ObjTypes(M, New, Array);
    ObjTypes(M, Get, Field);
    ObjTypes(M, Set, Field);
    ObjTypes(M, GetStatic, Field);
    ObjTypes(M, SetStatic, Field);

    #undef ObjTypesV
    #undef ObjTypes
    #undef N
    #undef M

    return i;
}
