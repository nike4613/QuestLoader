#include "jit/jit.hpp"
#include "log.hpp"

#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

using namespace modloader::jit;

namespace {
    auto pageSize = sysconf(_SC_PAGESIZE);
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
