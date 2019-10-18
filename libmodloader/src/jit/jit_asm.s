# 
# File containing stubs for JIT generated functions.
# Excellent resources:
#   https://modexp.wordpress.com/2018/10/30/arm64-assembly/ for A64 instructions,
#   "armclang reference guide" for .blah and other preprocessing stuff
# #x means the symbol x is immediate
#
fp .req x29

.arch armv8-a
.data
     /* ::modloader::jit::stubs::native_wrapper  */
.global _ZN9modloader3jit5stubs14native_wrapperE
.global _ZN9modloader3jit5stubs19native_wrapper_sizeE

.balign 32
native_wrapper: // can use r9-r15, r16, r17 safely
                # need to save r0-r7
    b #native_wrapper_body  // jump to body
    nop
.balign 8
target_addr: // this becomes exactly offset of 8
    # to be replaced with actual target address
    .quad 0x0  // declares a variable of 64-bits.
target_name:
    .quad 0x0
get_patched_env:
    .quad _ZN3jni9interface15get_patched_envEP7_JNIEnv
log_target:
    .quad 0x0
.balign 16 // this has to be aligned by 16 bytes?
.type native_wrapper_body, %function
native_wrapper_body:
    # set up stack frame
    stp fp,lr,[sp, #0x0]  // Store Pair of registers
    sub sp,sp,#0x60  // allocate 0x60 bytes of stack
    mov fp,sp

    # save parameters (there is probably a better way to do this)
    //in ARM, would do: push {r0-r7}
    stp x0, x1, [sp, #-0x40]
    stp x2, x3, [sp, #-0x30]
    stp x4, x5, [sp, #-0x20]
    stp x6, x7, [sp, #-0x10]

    # skip logging if log_target is null
    ldr x12, #log_target  // loads value at log_target into x12
    cbz x12, #post_log  // Compare & Branch on Zero (if X12 is 0, jump to post_log)

    # otherwise, call log_target
    ldr x0, #target_addr
    ldr x1, #target_name
    blr x12  // calls log_target

    b #post_log  // jump to post_log

.balign 16
post_log:
    ldp x0, x1, [sp, #-0x40]
    # call get_patched_env
    ldr x12, #get_patched_env  // load the contents of get_patched_env
    blr x12  // Branch with Link to Register (subroutine call to register contents)

    # x0 now has correct pointer, so restore the rest of the register arguments
    ldp x0, x1, [sp, #-0x40]
    ldp x2, x3, [sp, #-0x30]
    ldp x4, x5, [sp, #-0x20]
    ldp x6, x7, [sp, #-0x10]

    # fix frame pointer and stack pointer
    add sp,sp, #0x60
    ldp fp,lr, [sp, #0x0]

    ldr x12, #target_addr
    br x12 // tail call to target
.balign 4 // ensure that we don't accidentally leave off something
// GCC preprocessing magic
native_wrapper_len = (. - native_wrapper) / 4

_ZN9modloader3jit5stubs14native_wrapperE:
    .quad native_wrapper
_ZN9modloader3jit5stubs19native_wrapper_sizeE:
    .quad native_wrapper_len
