#pragma once

#include "modloader/_config.hpp"
#include <cstddef>
#include <span>

namespace modloader::jit {

    namespace mem {
        enum class protection {
            none = 0,
            read = 0b001, write = 0b010, execute = 0b100,
            read_write = read | write,
            read_execute = read | execute,
            write_execute = write | execute, // usually disallowed by the system
            read_write_execute = read | write | execute
        };

        int operator&(protection, protection) noexcept;

        // returns 0 or errno
        // WARNING: THIS WILL ALIGN THE POINTER TO THE NEXT PAGE BOUNDARY BEFORE AND CHANGE **ALL** OF IT
        int protect(void*, size_t, protection) noexcept;
        template<typename T>
        int MODLOADER_HIDE protect(T* data, size_t count, protection prot) noexcept
        { return protect(reinterpret_cast<void*>(data), count*sizeof(T), prot); }
        template<typename T, size_t N>
        int MODLOADER_HIDE protect(T (&data)[N], protection prot) noexcept
        { return protect(data, N, prot); }
        template<typename T>
        int MODLOADER_HIDE protect(std::span<T> data, protection prot) noexcept
        { return protect(data.data(), data.size(), prot); }
        template<typename T, std::ptrdiff_t N>
        int MODLOADER_HIDE protect(std::span<T, N> data, protection prot) noexcept
        { return protect(data, N, prot); }
    }
    

}