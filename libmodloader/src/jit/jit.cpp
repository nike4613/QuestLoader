#include "jit/jit.hpp"
#include "log.hpp"

#include <vector>
#include <span>

using namespace modloader::jit;

namespace modloader::jit::stubs {
    // using uint32_t is safe because each instruction is 32 bits
    extern uint32_t const* native_wrapper;
    extern size_t native_wrapper_size;
}

namespace {
    struct native_wrapper_start {
        // there are 2 instructions before the pointers
        uint32_t const begin[2];
        void* target;
        char const* target_name;
        void*const get_patched_env;
        void(*log_target)(void* target, char const* name) noexcept;
    };

    native_wrapper_start* from_wrapper(uint32_t* wrap) noexcept {
        return reinterpret_cast<native_wrapper_start*>(wrap);
    }

    void wrapper_log(void* target, char const* name) noexcept {
        logf(ANDROID_LOG_DEBUG, "Wrapper function for %s (%p) invoked", name ? name : "(nullptr)", target);
    }
}

void* modloader::jit::make_native_wrapper(void* original, char const* target_name) noexcept {
    std::span wrapTempl {stubs::native_wrapper, 
                         static_cast<std::ptrdiff_t>(stubs::native_wrapper_size)};
                    //   the standard says the above *should* be std::size_t not std::ptrdiff_t
    std::span wrap {new(mem::aligned, 16) uint32_t[wrapTempl.size()], wrapTempl.size()};
    std::copy(wrapTempl.begin(), wrapTempl.end(), wrap.begin());

    from_wrapper(wrap.data())->target = original;
    from_wrapper(wrap.data())->target_name = target_name;
    from_wrapper(wrap.data())->log_target = target_name ? &wrapper_log : nullptr;

    int success = mem::protect(wrap, mem::protection::read_write_execute);

    logf(ANDROID_LOG_DEBUG, "Generating native wrapper around %p at %p (prot returned %d)", original, wrap.data(), success);
    logf(ANDROID_LOG_DEBUG, "Log function is %p, name %s", from_wrapper(wrap.data())->log_target, target_name ? target_name : "(nullptr)");

    return wrap.data();
}