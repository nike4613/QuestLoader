#include "jit/jit.hpp"
#include "log.hpp"

#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <span>

using namespace modloader::jit;

namespace modloader::jit::stubs {
    // using uint32_t is safe because each instruction is 32 bits
    extern uint32_t const* native_wrapper;
    extern size_t native_wrapper_size;
}

namespace {
    auto pageSize = sysconf(_SC_PAGESIZE);

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

int modloader::jit::mem::operator&(protection a, protection b) noexcept {
    return static_cast<int>(a) & static_cast<int>(b);
}

int modloader::jit::mem::protect(void* data, size_t size, protection prot) noexcept {
    int mprot = PROT_NONE;
    if (prot & protection::read)
        mprot |= PROT_READ;
    if (prot & protection::write)
        mprot |= PROT_WRITE;
    if (prot & protection::execute)
        mprot |= PROT_EXEC;

    auto ptrs = reinterpret_cast<size_t>(data);
    auto diff = ptrs % pageSize;
    ptrs -= diff;

    auto ret = mprotect(reinterpret_cast<void*>(ptrs), size + diff, mprot);
    if (ret != 0) return errno;
    else return 0;
}

void* modloader::jit::make_native_wrapper(void* original, char const* target_name) noexcept {
    std::span wrapTempl {stubs::native_wrapper, 
                         static_cast<std::ptrdiff_t>(stubs::native_wrapper_size)};
                    //   the standard says the above *should* be std::size_t not std::ptrdiff_t
    std::span wrap {new uint32_t[wrapTempl.size()], wrapTempl.size()};
    std::copy(wrapTempl.begin(), wrapTempl.end(), wrap.begin());

    from_wrapper(wrap.data())->target = original;
    from_wrapper(wrap.data())->target_name = target_name;
    from_wrapper(wrap.data())->log_target = target_name ? &wrapper_log : nullptr;

    int success = mem::protect(wrap, mem::protection::read_write_execute);

    logf(ANDROID_LOG_DEBUG, "Generating native wrapper around %p at %p (prot returned %d)", original, wrap.data(), success);
    logf(ANDROID_LOG_DEBUG, "Log function is %p, name %s", from_wrapper(wrap.data())->log_target, target_name ? target_name : "(nullptr)");

    return wrap.data();
}