/* tinynew.cpp
   
   Overrides operators new and delete
   globally to reduce code size.
   
   Public domain, use however you wish.
   If you really need a license, consider it MIT:
   http://www.opensource.org/licenses/mit-license.php
   
   - Eric Agan
     Elegant Invention
 */

#include <malloc.h>
#include "log.hpp"
#include "modloader/mem.hpp"

[[nodiscard]]
void* operator new(std::size_t size) {
    return malloc(size);
}
 
[[nodiscard]]
void* operator new[](std::size_t size) {
    return malloc(size);
}
 
void operator delete(void* ptr) {
    free(ptr);
}
 
void operator delete[](void* ptr) {
    free(ptr);
}
 
/* Optionally you can override the 'nothrow' versions as well.
   This is useful if you want to catch failed allocs with your
   own debug code, or keep track of heap usage for example,
   rather than just eliminate exceptions.
 */
 
[[nodiscard]]
void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
    return malloc(size);
}
 
[[nodiscard]]
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
    return malloc(size);
}
 
void operator delete(void* ptr, const std::nothrow_t&) noexcept {
    free(ptr);
}
 
void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
    free(ptr);
}

/* 
    Aligned new/delete
*/

[[nodiscard]]
void* operator new(std::size_t size, std::align_val_t align) {
    return memalign(static_cast<std::size_t>(align), size);
}
 
[[nodiscard]]
void* operator new[](std::size_t size, std::align_val_t align) {
    return memalign(static_cast<std::size_t>(align), size);
}
 
void operator delete(void* ptr, std::align_val_t align) noexcept {
    free(ptr);
}
 
void operator delete[](void* ptr, std::align_val_t align) noexcept {
    free(ptr);
}

/*
    Additional stuff to help reduce binary size
*/
extern "C" void __cxa_pure_virtual() {
    log(ANDROID_LOG_ERROR, "Pure virtual function call");
    while (1);
}