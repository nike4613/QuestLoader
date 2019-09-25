# 
# File containing stubs for JIT generated functions
#

.data
     /* ::modloader::jit::stubs::native_wrapper  */
.global _ZN9modloader3jit5stubs14native_wrapperE
.global _ZN9modloader3jit5stubs19native_wrapper_sizeE

.align 4
native_wrapper: // can use r9-r15, r16, r17 safely
                # need to save r0-r8
    b #native_wrapper_body
    nop
target_addr: // this becomes exactly offset of 8
    # to be replaced with actual target address
    .quad 0x0123456789abcdef
target_name:
    .quad 0x0
get_patched_env:
    .quad _ZN3jni9interface15get_patched_envEP7_JNIEnv
log_target:
    .quad 0x0
.align 4 // this needs to be 16-byte aligned it seems
native_wrapper_body:
    # set up stack frame
    stp fp,lr,[sp, #0x0]
    sub sp,sp,#0x60
    mov fp,sp

    # save parameters (there is probably a better way to do this)
    stp x1, x2, [sp, #-0x40]
    stp x3, x4, [sp, #-0x30]
    stp x5, x6, [sp, #-0x20]
    stp x7, x8, [sp, #-0x10]
    //push {r1-r7} // save parameters 

    ldr x16, log_target
    cmp x16, #0
    b.eq #nolog

    str x0, [sp, #-0x50]

    ldr x16, log_target
    ldr x0, target_addr
    ldr x1, target_name
    blr x16

    ldr x0, [sp, #-0x50]
    b nolog
    
    .align 4
nolog:
    # call get_patched_env
    ldr x16, get_patched_env
    blr x16

    # x0 now has correct pointer, so restore the rest of the register arguments
    ldp x1, x2, [sp, #-0x40]
    ldp x3, x4, [sp, #-0x30]
    ldp x5, x6, [sp, #-0x20]
    ldp x7, x8, [sp, #-0x10]

    # fix frame pointer and stack pointer
    add sp,sp, #0x60
    ldp fp,lr, [sp, #0x0]

    ldr x16, target_addr
    br x16 // tail call to target
    .align 2 // ensure that we son't accidentally leave off something
native_wrapper_len = (. - native_wrapper) / 4

_ZN9modloader3jit5stubs14native_wrapperE:
    .quad native_wrapper
_ZN9modloader3jit5stubs19native_wrapper_sizeE:
    .quad native_wrapper_len
