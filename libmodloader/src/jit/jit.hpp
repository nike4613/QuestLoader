#pragma once

#include "modloader/_config.hpp"
#include "modloader/mem.hpp"
#include <cstddef>

namespace modloader::jit {
    
    void* make_native_wrapper(void* original, char const* target_name = nullptr) noexcept;

}