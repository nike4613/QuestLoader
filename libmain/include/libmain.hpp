#ifndef LIBMAIN_HPP
#define LIBMAIN_HPP

#include <jni.h>

namespace jni::NativeLoader {

    jboolean load(JNIEnv* env, jobject klass, jstring str) noexcept;
    jboolean unload(JNIEnv* env, jobject klass) noexcept;

}

#endif